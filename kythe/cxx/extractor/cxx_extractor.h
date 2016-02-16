/*
 * Copyright 2014 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef KYTHE_CXX_EXTRACTOR_EXTRACTOR_H_
#define KYTHE_CXX_EXTRACTOR_EXTRACTOR_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "clang/Tooling/Tooling.h"
#include "glog/logging.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/cxx_details.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/index_pack.h"
#include "kythe/proto/analysis.pb.h"

namespace clang {
class FrontendAction;
class FileManager;
}

namespace kythe {

/// \brief An opaque representation of the behavior of the preprocessor.
///
/// The extractor collects logs of the observable behavior of the preprocessor
/// called transcripts. Observable behavior includes operations like macro
/// expansion or the selection of a branch during conditional compilation.
/// We use these transcripts to determine when a particular preprocessor context
/// is observationally equivalent to another. For example, if `a.h` is used in
/// two contexts, one in which another (independent) header has been included
/// and one in which it has not, those contexts should be equivalent modulo
/// `a.h`.
///
/// See //kythe/cxx/indexer/cxx/claiming.ad for more details.
using PreprocessorTranscript = std::string;

/// \brief Describes special handling directives for claiming a resource.
enum class ClaimDirective {
  NoDirectivesFound,  ///< No directives were issued.
  AlwaysClaim         ///< This resource should always be claimed.
};

/// \brief A record for a single source file.
struct SourceFile {
  std::string file_content;  ///< The full uninterpreted file content.
  struct FileHandlingAnnotations {
    ClaimDirective default_claim;  ///< Claiming behavior for this version.
    /// The (include-#-offset, that-version) components of the tuple set
    /// described below.
    std::map<unsigned, PreprocessorTranscript> out_edges;
  };
  /// A set of tuples (this-version, include-#-offset, that-version) such that
  /// if we are in file this-version and reach an include at
  /// include-#-offset, we can expect to enter another file that-version.
  /// The offset is in number of bytes from the start of the file.
  std::map<PreprocessorTranscript, FileHandlingAnnotations> include_history;
  /// This SourceFile's vname, normalized according to the configuration file.
  kythe::proto::VName vname;
};

/// \brief A function the extractor will call once it's done extracting input
/// for a particular `main_source_file`.
/// \param main_source_file The path used by Clang to refer to the main source
/// file for this compilation action.
/// \param main_source_file_transcript The transcript for this main_source_file.
/// Depending on the interesting preprocessor definitions made in the
/// environment, this might differ between compilation units.
/// \param source_files All files, including the `main_source_file`, that will
/// be touched during the compilation action. The keys are the paths used by
/// Clang to refer to each file.
/// \param header_search_info The header search information to use (or null
/// if none).
/// \param had_errors Whether we encountered any errors so far.
using ExtractorCallback = std::function<void(
    const std::string &main_source_file,
    const PreprocessorTranscript &main_source_file_transcript,
    const std::unordered_map<std::string, SourceFile> &source_files,
    const HeaderSearchInfo *header_search_info, bool had_errors)>;

/// \brief Called by the `IndexWriter` once it has finished building protobufs.
///
/// Generally writes them out to a file, but may retain them for testing.
class IndexWriterSink {
 public:
  /// \brief Called before `WriteHeader`.
  /// \param path The path to which the index should be written.
  /// \param unit_hash The identifier for the compilation unit being written.
  virtual void OpenIndex(const std::string &path,
                         const std::string &unit_hash) = 0;
  /// \brief Writes the `CompilationUnit` to the index.
  virtual void WriteHeader(const kythe::proto::CompilationUnit &header) = 0;
  /// \brief Writes a `FileData` record to the indexfile.
  virtual void WriteFileContent(const kythe::proto::FileData &content) = 0;
  virtual ~IndexWriterSink() {}
};

/// \brief Writes extracted data to an index pack.
class IndexPackWriterSink : public IndexWriterSink {
 public:
  void OpenIndex(const std::string &path,
                 const std::string &unit_hash) override;
  void WriteHeader(const kythe::proto::CompilationUnit &header) override;
  void WriteFileContent(const kythe::proto::FileData &content) override;

 private:
  /// The open index pack, if any.
  std::unique_ptr<IndexPack> pack_;
};

/// \brief An `IndexWriterSink` that writes to physical .kindex files.
class KindexWriterSink : public IndexWriterSink {
 public:
  /// \param force_path If nonempty, will always write to this file.
  explicit KindexWriterSink(const std::string &force_path)
      : force_path_(force_path) {}
  void OpenIndex(const std::string &path,
                 const std::string &unit_hash) override;
  void WriteHeader(const kythe::proto::CompilationUnit &header) override;
  void WriteFileContent(const kythe::proto::FileData &content) override;
  ~KindexWriterSink();

 private:
  /// The file descriptor in use, opened in `OpenIndex` and closed in the dtor
  /// after `file_stream_` is destroyed. Owned by this object.
  int fd_ = -1;
  /// Wraps `fd_`. Destroyed after `gzip_stream_`.
  std::unique_ptr<google::protobuf::io::FileOutputStream> file_stream_;
  /// Wraps `file_stream_`. Destroyed after `coded_stream_`.
  std::unique_ptr<google::protobuf::io::GzipOutputStream> gzip_stream_;
  /// Wraps `gzip_stream_`. Destroyed first in the destructor.
  std::unique_ptr<google::protobuf::io::CodedOutputStream> coded_stream_;
  /// The path to the file whose handle is held by `fd_`.
  std::string open_path_;
  /// If nonempty, the path to use.
  std::string force_path_;
};

/// \brief Collects information about compilation arguments and targets and
/// writes it to an index file.
class IndexWriter {
 public:
  /// \brief Set the arguments to be used for this compilation.
  ///
  /// `args` should be the `argv` (without terminating null) that would be
  /// passed to the main() of a build tool. It includes both the tool's
  /// name as it was invoked and the name of the main source file.
  void set_args(const std::vector<std::string> &args) { args_ = args; }
  /// \brief Configure the default corpus.
  void set_corpus(const std::string &corpus) { corpus_ = corpus; }
  /// \brief Configure vname generation using some JSON string.
  /// \return true on success, false on failure
  bool SetVNameConfiguration(const std::string &json_string);
  /// \brief Configure where the indexer will output files.
  void set_output_directory(const std::string &dir) { output_directory_ = dir; }
  /// \brief Configure the path used for the root.
  void set_root_directory(const std::string &dir) { root_directory_ = dir; }
  const std::string &root_directory() const { return root_directory_; }
  /// \brief Write the index file to `sink`, consuming the sink in the process.
  void WriteIndex(
      std::unique_ptr<IndexWriterSink> sink,
      const std::string &main_source_file, const std::string &entry_context,
      const std::unordered_map<std::string, SourceFile> &source_files,
      const HeaderSearchInfo *header_search_info, bool had_errors,
      const std::string &clang_working_dir);
  /// \brief Set the fields of `file_input` for the given file.
  /// \param clang_path A path to the file as seen by clang.
  /// \param source_file The `SourceFile` to configure `file_input` with.
  /// \param file_input The proto to configure.
  void FillFileInput(const std::string &clang_path,
                     const SourceFile &source_file,
                     kythe::proto::CompilationUnit_FileInput *file_input);

  /// \brief Attempts to generate a VName for the file at some path.
  /// \param path The path (likely from Clang) to the file.
  kythe::proto::VName VNameForPath(const std::string &path);

 private:
  /// The `FileVNameGenerator` used to generate file vnames.
  FileVNameGenerator vname_generator_;
  /// The arguments used for this compilation.
  std::vector<std::string> args_;
  /// The default corpus to use for artifacts.
  std::string corpus_ = "";
  /// The directory to use for index files.
  std::string output_directory_ = ".";
  /// The directory to use to generate relative paths.
  std::string root_directory_ = ".";
};

/// \brief Creates a `FrontendAction` that records information about a
/// compilation involving a single source file and all of its dependencies.
/// \param index_writer The `IndexWriter` to use.
/// \param callback A function to call once extraction is complete.
std::unique_ptr<clang::FrontendAction> NewExtractor(IndexWriter *index_writer,
                                                    ExtractorCallback callback);

/// \brief Adds builtin versions of the compiler header files to
/// `invocation`'s virtual file system in `map_directory`.
/// \param invocation The invocation to modify.
/// \param map_directory The directory to use.
void MapCompilerResources(clang::tooling::ToolInvocation *invocation,
                          const char *map_directory);

/// \brief Contains the configuration necessary for the extractor to run.
class ExtractorConfiguration {
 public:
  /// \brief Set the arguments that will be passed to Clang.
  void SetArgs(const std::vector<std::string> &args);
  /// \brief Initialize the configuration using the process environment.
  void InitializeFromEnvironment();
  /// \brief Load the VName config file from `path` or terminate.
  void SetVNameConfig(const std::string &path);
  /// \brief If a kindex file will be written, write it here.
  void SetKindexOutputFile(const std::string &path) { kindex_path_ = path; }
  /// \brief Execute the extractor with this configuration.
  void Extract();

 private:
  /// The argument list to pass to Clang.
  std::vector<std::string> final_args_;
  /// The FileSystemOptions to use during extraction.
  clang::FileSystemOptions file_system_options_;
  /// The IndexWriter to use.
  IndexWriter index_writer_;
  /// True if we should use our internal system headers; false if not.
  bool map_builtin_resources_ = true;
  /// True if we should use index packs; false if not.
  bool using_index_packs_ = false;
  /// If nonempty, emit kindex files to this exact path.
  std::string kindex_path_;
};

}  // namespace kythe

#endif
