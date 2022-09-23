/*
 * Copyright 2016 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_INDEXER_CXX_FRONTEND_H_
#define KYTHE_CXX_INDEXER_CXX_FRONTEND_H_

#include <string>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "kythe/cxx/common/indexing/KytheCachingOutput.h"
#include "kythe/cxx/indexer/cxx/KytheClaimClient.h"
#include "kythe/proto/analysis.pb.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace kythe {

/// \brief A compilation unit to be indexed.
struct IndexerJob {
  /// All files necessary for the compilation under analysis.
  std::vector<proto::FileData> virtual_files;
  /// The compilation under analysis.
  proto::CompilationUnit unit;
  /// If set, this job should not produce any output.
  bool silent = false;
};

/// \brief Handles common tasks related to invoking a Kythe indexer from the
/// command line.
class IndexerContext {
 public:
  /// \param args Arguments to the indexer (after removing any flags).
  /// \param default_filename Filename to use when reading from stdin
  /// (e.g., "stdin.cc")
  /// \pre google::ParseCommandLineFlags has been called.
  IndexerContext(const std::vector<std::string>& args,
                 const std::string& default_filename);
  ~IndexerContext();

  /// \brief A callback used when enumerating compilations.
  using CompilationVisitCallback = std::function<void(IndexerJob&)>;
  /// \brief Reads compilations from the file(s) specified in the constructor
  /// and calls the callback for each one.
  /// \param visit A callback to call for each compilation unit.
  void EnumerateCompilations(const CompilationVisitCallback& visit);

  /// \brief If non-null, the hash cache to use. Owned by `IndexerContext`.
  HashCache* hash_cache() const { return hash_cache_.get(); }
  /// \brief If true, the indexer is permitted to touch the local filesystem.
  bool allow_filesystem_access() const {
    // Only allow filesystem access for unpacked inputs. Indexes already contain
    // all the necessary files.
    return unpacked_inputs_;
  }
  /// \brief If true, the indexer should handle unknown elements gracefully.
  bool ignore_unimplemented() const { return ignore_unimplemented_; }
  /// \brief The claim client to use for this compilation. Not null.
  KytheClaimClient* claim_client() const {
    CHECK(claim_client_ != nullptr);
    return claim_client_.get();
  }
  /// \brief The output stream to use for this compilation. Not null; owned
  /// by `IndexerContext` and closed on destruction.
  FileOutputStream* output() const {
    CHECK(kythe_output_ != nullptr);
    return kythe_output_.get();
  }

  /// \brief Generates a usage message for this indexer.
  /// \param program_title a description of the indexer
  /// ("the Kythe C++ indexer")
  /// \param program_name the executable name of the indexer ("cxx_indexer")
  static std::string UsageMessage(const std::string& program_title,
                                  const std::string& program_name);

 private:
  /// \brief Checks to see if a kzip was specified.
  /// \return true if there are kzip arguments; false otherwise.
  bool HasIndexArguments();
  /// \brief Loads from a kzip file
  /// \param file_or_cu The name of the .kzip (with extension) or the
  /// compilation unit hash.
  /// \param visit A callback to call for each compilation unit.
  void LoadDataFromKZip(const std::string& file_or_cu,
                        const CompilationVisitCallback& visit);
  /// \brief Load data from an unpacked file.
  /// \param default_filename The filename to use if we're reading from stdin.
  /// \param visit A callback to call for each compilation unit.
  void LoadDataFromUnpackedFile(const std::string& default_filename,
                                const CompilationVisitCallback& visit);
  /// \brief Initialize a claim client.
  void InitializeClaimClient();
  /// \brief Prepare to write to output.
  void OpenOutputStreams();
  /// \brief Flush output.
  void CloseOutputStreams();
  /// \brief Configure the hash cache (if one was requested).
  void OpenHashCache();

  /// Command-line arguments, pruned of empty strings and gflags.
  std::vector<std::string> args_;
  /// Default_filename The filename to use if we're reading from stdin.
  std::string default_filename_;
  /// The file descriptor to which we're writing output.
  int write_fd_ = -1;
  /// Wraps `write_fd_`.
  std::unique_ptr<google::protobuf::io::FileOutputStream> raw_output_;
  /// Wraps `raw_output_`.
  std::unique_ptr<FileOutputStream> kythe_output_;
  /// The claim client to use during analysis.
  std::unique_ptr<kythe::KytheClaimClient> claim_client_;
  /// The hash cache to use during analysis (or null).
  std::unique_ptr<HashCache> hash_cache_;
  /// Whether the args specify an unpacked input file as opposed to an index.
  bool unpacked_inputs_ = false;
  /// Whether to ignore missing cases during analysis.
  bool ignore_unimplemented_ = false;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_FRONTEND_H_
