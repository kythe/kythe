/*
 * Copyright 2016 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_FRONTEND_H_
#define KYTHE_CXX_COMMON_FRONTEND_H_

#include <string>
#include <vector>

#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "kythe/cxx/common/index_pack.h"
#include "kythe/cxx/common/indexing/KytheClaimClient.h"
#include "kythe/cxx/common/indexing/KytheOutputStream.h"
#include "kythe/proto/analysis.pb.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace kythe {

/// \brief Handles common tasks related to invoking a Kythe indexer from the
/// command line.
class IndexerContext {
 public:
  /// \param args Arguments to the indexer (after removing any flags).
  /// \param default_filename Filename to use when reading from stdin
  /// (e.g., "stdin.cc")
  /// \pre google::ParseCommandLineFlags has been called.
  IndexerContext(const std::vector<std::string> &args,
                 const std::string &default_filename);
  ~IndexerContext();

  /// \brief If non-null, the hash cache to use. Owned by `IndexerContext`.
  HashCache *hash_cache() const { return hash_cache_.get(); }
  /// \brief If true, the indexer is permitted to touch the local filesystem.
  bool allow_filesystem_access() const { return allow_filesystem_access_; }
  /// \brief If true, the indexer may make decisions about claiming that could
  /// lose data.
  bool enable_lossy_claiming() const { return enable_lossy_claiming_; }
  /// \brief The absolute working directory in which indexing is taking place.
  /// This may not exist on the local filesystem.
  const std::string &working_directory() const { return working_dir_; }
  /// \brief If true, the indexer should handle unknown elements gracefully.
  bool ignore_unimplemented() const { return ignore_unimplemented_; }
  /// \brief All files necessary for the compilation being analyzed.
  std::vector<proto::FileData> *virtual_files() { return &virtual_files_; }
  /// \brief The compilation being analyzed.
  const proto::CompilationUnit &unit() const { return unit_; }
  /// \brief The claim client to use for this compilation. Not null.
  KytheClaimClient *claim_client() const {
    CHECK(claim_client_ != nullptr);
    return claim_client_.get();
  }
  /// \brief The output stream to use for this compilation. Not null; owned
  /// by `IndexerContext` and closed on destruction.
  FileOutputStream *output() const {
    CHECK(kythe_output_ != nullptr);
    return kythe_output_.get();
  }

  /// \brief Generates a usage message for this indexer.
  /// \param program_title a description of the indexer
  /// ("the Kythe C++ indexer")
  /// \param program_name the executable name of the indexer ("cxx_indexer")
  static std::string UsageMessage(const std::string &program_title,
                                  const std::string &program_name);

 private:
  /// \brief Checks to see if a .kindex or index pack was specified.
  /// \return The name of the .kindex (with extension) or the compilation unit
  /// hash (if either was specified); otherwise, an empty string.
  std::string CheckForIndexArguments();
  /// \brief Loads from an index pack or .kindex.
  /// \param kindex_file_or_cu The name of the .kindex (with extension) or
  /// the compilation unit hash.
  void LoadDataFromIndex(const std::string &kindex_file_or_cu);
  /// \brief Load data from an unpacked file.
  /// \param default_filename The filename to use if we're reading from stdin.
  void LoadDataFromUnpackedFile(const std::string &default_filename);
  /// \brief Initialize a claim client.
  void InitializeClaimClient();
  /// \brief Normalize input file vnames by cleaning paths and clearing
  /// signatures.
  void NormalizeFileVNames();
  /// \brief Prepare to write to output.
  void OpenOutputStreams();
  /// \brief Flush output.
  void CloseOutputStreams();
  /// \brief Configure the hash cache (if one was requested).
  void OpenHashCache();

  /// Command-line arguments, pruned of empty strings and gflags.
  std::vector<std::string> args_;
  /// All files necessary for the compilation under analysis.
  std::vector<proto::FileData> virtual_files_;
  /// The compilation under analysis.
  proto::CompilationUnit unit_;
  /// The absolute virtual directory in which analysis occurs.
  std::string working_dir_;
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
  /// Whether access to the local filesystem is allowed during analysis.
  bool allow_filesystem_access_ = false;
  /// Whether possibly lossy optimizations are allowed.
  bool enable_lossy_claiming_ = false;
  /// Whether to ignore missing cases during analysis.
  bool ignore_unimplemented_ = false;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_FRONTEND_H_
