/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>

#include "absl/log/log.h"
#include "clang/Tooling/Tooling.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/gzip_stream.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/common/file_vname_generator.h"
#include "kythe/cxx/common/index_writer.h"
#include "kythe/cxx/common/path_utils.h"
#include "kythe/cxx/extractor/cxx_details.h"
#include "kythe/cxx/extractor/language.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/filecontext.pb.h"

namespace clang {
class FrontendAction;
class FileManager;
}  // namespace clang

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
    const std::string& main_source_file,
    const PreprocessorTranscript& main_source_file_transcript,
    const std::unordered_map<std::string, SourceFile>& source_files,
    const HeaderSearchInfo* header_search_info, bool had_errors)>;

/// \brief Called by the `CompilationWriter` once it has finished building
/// protobufs.
///
/// Generally writes them out to a file, but may retain them for testing.
class CompilationWriterSink {
 public:
  /// \brief Called before `WriteHeader`.
  /// \param unit_hash The identifier for the compilation unit being written.
  virtual void OpenIndex(const std::string& unit_hash) = 0;
  /// \brief Writes the `CompilationUnit` to the index.
  virtual void WriteHeader(const kythe::proto::CompilationUnit& header) = 0;
  /// \brief Writes a `FileData` record to the indexfile.
  virtual void WriteFileContent(const kythe::proto::FileData& content) = 0;
  virtual ~CompilationWriterSink() = default;
};

/// \brief A `CompilationWriterSink` which writes to .kzip files.\
/// See https://www.kythe.io/docs/kythe-kzip.html for a description.
class KzipWriterSink : public CompilationWriterSink {
 public:
  enum class OutputPathType {
    Directory,
    SingleFile,
  };
  /// \param path The file to which to write.
  /// \param path_type If SingleFile, the kzip is written to the specified path
  /// directly. Otherwise the path is interpreted as a directory and the kzip is
  /// written within it using a filename derived from an identifying hash of the
  /// compilation unit.
  explicit KzipWriterSink(const std::string& path, OutputPathType path_type);
  void OpenIndex(const std::string& unit_hash) override;
  void WriteHeader(const kythe::proto::CompilationUnit& header) override;
  void WriteFileContent(const kythe::proto::FileData& file) override;
  ~KzipWriterSink() override;

 private:
  std::string path_;
  OutputPathType path_type_;
  std::optional<IndexWriter> writer_;
};

/// \brief Collects information about compilation arguments and targets and
/// writes it to an index file.
class CompilationWriter {
 public:
  /// \brief Set the arguments to be used for this compilation.
  ///
  /// `args` should be the `argv` (without terminating null) that would be
  /// passed to the main() of a build tool. It includes both the tool's
  /// name as it was invoked and the name of the main source file.
  void set_args(const std::vector<std::string>& args) { args_ = args; }
  /// \brief Set the target triple used during compilation.
  ///
  /// Setting this allows the indexer to set the same triple that was used
  /// during extraction even if it is run on a machine with a different
  /// architecture.
  void set_triple(const std::string& triple) { triple_ = triple; }
  /// \brief Configure the default corpus.
  void set_corpus(const std::string& corpus) { corpus_ = corpus; }
  /// \brief Record the name of the target that generated this compilation.
  void set_target_name(const std::string& target) { target_name_ = target; }
  /// \brief Record the rule type that generated this compilation.
  void set_rule_type(const std::string& rule_type) { rule_type_ = rule_type; }
  /// \brief Record the build config targeted by this compilation.
  void set_build_config(const std::string& build_config) {
    build_config_ = build_config;
  }
  /// \brief Record the output path generated by this compilation.
  void set_output_path(const std::string& path) { output_path_ = path; }
  /// \brief Configure vname generation using some JSON string.
  /// \return true on success, false on failure
  bool SetVNameConfiguration(const std::string& json_string);
  /// \brief Configure the path used for the root.
  void set_root_directory(const std::string& dir) {
    canonicalizer_.reset();
    root_directory_ = dir;
  }
  const std::string& root_directory() const { return root_directory_; }

  /// \brief Configure the path canonicalization configuration.
  void set_path_canonicalization_policy(PathCanonicalizer::Policy policy) {
    canonicalizer_.reset();
    path_policy_ = policy;
  }
  /// \brief Don't include empty directories.
  void set_exclude_empty_dirs(bool exclude) { exclude_empty_dirs_ = exclude; }
  /// \brief Don't include files read during autoconfiguration.
  void set_exclude_autoconfiguration_files(bool exclude) {
    exclude_autoconfiguration_files_ = exclude;
  }
  /// \brief Write the index file to `sink`, consuming the sink in the process.
  void WriteIndex(
      supported_language::Language lang,
      std::unique_ptr<CompilationWriterSink> sink,
      const std::string& main_source_file, const std::string& entry_context,
      const std::unordered_map<std::string, SourceFile>& source_files,
      const HeaderSearchInfo* header_search_info, bool had_errors);
  /// \brief Set the fields of `file_input` for the given file.
  /// \param clang_path A path to the file as seen by clang.
  /// \param source_file The `SourceFile` to configure `file_input` with.
  /// \param file_input The proto to configure.
  void FillFileInput(const std::string& clang_path,
                     const SourceFile& source_file,
                     kythe::proto::CompilationUnit_FileInput* file_input);
  /// \brief Erases previously-recorded opened files (e.g., because they were
  /// used during autoconfiguration and are uninteresting).
  ///
  /// We will eventually want to replace this with a filter that matches against
  /// files whose paths are significant (like CUDA directories).
  void CancelPreviouslyOpenedFiles();

  /// \brief Erases previously-recorded paths to intermediate files.
  void ScrubIntermediateFiles(const clang::HeaderSearchOptions& options);

  /// \brief Records that a path was successfully opened for reading.
  void OpenedForRead(const std::string& clang_path);

  /// \brief Records that a directory path was successfully opened for status.
  void DirectoryOpenedForStatus(const std::string& clang_path);

  // A "strong" alias to differentiate filesystem paths from "root" paths.
  struct RootPath : std::tuple<std::string> {
    const std::string& value() const& { return std::get<0>(*this); }
    std::string& value() & { return std::get<0>(*this); }
    std::string&& value() && { return std::move(std::get<0>(*this)); }
  };
  /// \brief Attempts to generate a root-relative path.
  /// This is a path relative to KYTHE_ROOT_DIRECTORY, not the working directory
  /// and should only be used for doing VName mapping a lookups.
  RootPath RootRelativePath(absl::string_view path);

  /// \brief Attempts to generate a VName for the file at some path.
  /// \param path The path (likely from Clang) to the file.
  kythe::proto::VName VNameForPath(absl::string_view path);
  kythe::proto::VName VNameForPath(const RootPath& path);

 private:
  /// Called to read and insert content for extra include files.
  void InsertExtraIncludes(kythe::proto::CompilationUnit* unit,
                           kythe::proto::CxxCompilationUnitDetails* details);
  /// The `FileVNameGenerator` used to generate file vnames.
  FileVNameGenerator vname_generator_;
  /// The arguments used for this compilation.
  std::vector<std::string> args_;
  /// The host triple used during compilation
  std::string triple_ = "";
  /// The default corpus to use for artifacts.
  std::string corpus_ = "";
  /// The directory to use to generate relative paths.
  std::string root_directory_ = ".";
  /// The policy to use when generating relative paths.
  PathCanonicalizer::Policy path_policy_ =
      PathCanonicalizer::Policy::kCleanOnly;
  /// If nonempty, the name of the target that generated this compilation.
  std::string target_name_;
  /// If nonempty, the rule type that generated this compilation.
  std::string rule_type_;
  /// If nonempty, the output path generated by this compilation.
  std::string output_path_;
  /// If nonempty, the build configuration targeted by this compilation.
  std::string build_config_;
  /// Paths opened through the VFS that may not have been opened through the
  /// preprocessor.
  std::set<std::string> extra_includes_;
  /// Paths queried for status through the VFS.
  std::set<std::string> status_checked_paths_;
  /// FileData for those extra_includes_ that are actually necessary.
  std::vector<kythe::proto::FileData> extra_data_;
  /// Don't include empty directories.
  bool exclude_empty_dirs_ = false;
  /// Don't include files read during the autoconfiguration phase.
  bool exclude_autoconfiguration_files_ = false;

  /// The canonicalizer to use when constructing relative paths.
  /// Lazily built from policy and root above.
  std::optional<PathCanonicalizer> canonicalizer_;
};

/// \brief Creates a `FrontendAction` that records information about a
/// compilation involving a single source file and all of its dependencies.
/// \param index_writer The `CompilationWriter` to use.
/// \param callback A function to call once extraction is complete.
std::unique_ptr<clang::FrontendAction> NewExtractor(
    CompilationWriter* index_writer, ExtractorCallback callback);

/// \brief Adds builtin versions of the compiler header files to
/// `invocation`'s virtual file system in `map_directory`.
/// \param invocation The invocation to modify.
/// \param map_directory The directory to use.
void MapCompilerResources(clang::tooling::ToolInvocation* invocation,
                          const char* map_directory);

/// \brief Contains the configuration necessary for the extractor to run.
class ExtractorConfiguration {
 public:
  /// \brief Set the arguments that will be passed to Clang.
  void SetArgs(const std::vector<std::string>& args);
  /// \brief Initialize the configuration using the process environment.
  void InitializeFromEnvironment();
  /// \brief Load the VName config file from `path` or terminate.
  void SetVNameConfig(const std::string& path);
  /// \brief If a kzip file will be written, write it here.
  void SetOutputFile(const std::string& path) { output_file_ = path; }
  /// \brief Record the name of the target that generated this compilation.
  void SetTargetName(const std::string& target) { target_name_ = target; }
  /// \brief Record the rule type that generated this compilation.
  void SetRuleType(const std::string& rule_type) { rule_type_ = rule_type; }
  /// \brief Record the build config targeted by this compilation.
  void SetBuildConfig(const std::string& build_config) {
    build_config_ = build_config;
  }
  /// \brief Record the output path produced by this compilation.
  void SetCompilationOutputPath(const std::string& path) {
    compilation_output_path_ = path;
  }
  /// \brief Sets the canonicalization policy to use for VName paths.
  void SetPathCanonizalizationPolicy(PathCanonicalizer::Policy policy) {
    index_writer_.set_path_canonicalization_policy(policy);
  }
  /// \brief Executes the extractor with this configuration, returning true on
  /// success.
  bool Extract(supported_language::Language lang);
  /// \brief Executes the extractor with this configuration to the provided
  /// sink, returning true on success.
  bool Extract(supported_language::Language lang,
               std::unique_ptr<CompilationWriterSink> sink);

 private:
  /// The argument list to pass to Clang.
  std::vector<std::string> final_args_;
  /// The CompilationWriter to use.
  CompilationWriter index_writer_;
  /// True if we should use our internal system headers; false if not.
  bool map_builtin_resources_ = true;
  /// The directory to use for index files.
  std::string output_directory_ = ".";
  /// If nonempty, emit kzip files to this exact path.
  std::string output_file_;
  /// If nonempty, the name of the target that generated this compilation.
  std::string target_name_;
  /// If nonempty, the rule type that generated this compilation.
  std::string rule_type_;
  /// If nonempty, the output path generated by this compilation.
  std::string compilation_output_path_;
  /// If nonempty, the name of the build config targeted by this compilation.
  std::string build_config_;
};

}  // namespace kythe

#endif
