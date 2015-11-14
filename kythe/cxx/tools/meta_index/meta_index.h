/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_TOOLS_META_INDEX_META_INDEX_H_
#define KYTHE_CXX_TOOLS_META_INDEX_META_INDEX_H_

#include "clang/Basic/VirtualFileSystem.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "kythe/cxx/tools/meta_index/commonjs_package_registry.h"
#include "kythe/cxx/tools/meta_index/vcs.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

namespace kythe {

class Repository {
 public:
  struct PathMatchingRule {
    /// The regex to use to identify this Repository by one of its files.
    std::string match_regex;
    /// The template string to use to build a VName corpus.
    std::string corpus_template;
    /// The template string to use to build a VName root. May be empty.
    std::string root_template;
    /// The template string to use to build a VName path.
    std::string path_template;
    /// The priority of this matching rule; higher priorities are matched
    /// first.
    size_t match_priority;
  };
  /// Returns a `PathMatchingRule` for this `Repository`.
  virtual PathMatchingRule path_matching_rule() const = 0;
  virtual ~Repository() {}
  /// \brief Emits Kythe Entries to describe this repository.
  virtual void EmitKytheSubgraph(
      google::protobuf::io::ZeroCopyOutputStream *stream) const = 0;
  /// \brief Returns the arguments necessary to index the files in this
  /// repository.
  virtual std::vector<std::string> GetIndexArgs() const = 0;
  /// \brief Generate a `PathMatchingRule` that might just work.
  /// \param local_repo_path local absolute path to repository checkout
  /// \param vcs_type version control system type (e.g., "git")
  /// \param vcs_uri version control system URI, possibly to repo root at head
  /// \param vcs_id version control system revision ID
  /// \param vcs_path path from VCS root to this repository root
  /// \param match_priority priority for performing this match
  static PathMatchingRule GenerateGenericPathMatchingRule(
      const std::string &local_repo_path, const std::string &vcs_type,
      const std::string &vcs_uri, const std::string &vcs_id,
      const std::string &vcs_path, size_t match_priority);
};

inline bool operator<(const Repository::PathMatchingRule &lhs,
                      const Repository::PathMatchingRule &rhs) {
  return lhs.match_priority < rhs.match_priority;
}

class Repositories {
 public:
  /// \brief Builds a logical repository view on top of a filesystem.
  Repositories(llvm::IntrusiveRefCntPtr<clang::vfs::FileSystem> filesystem,
               std::unique_ptr<CommonJsPackageRegistryClient> registry,
               std::unique_ptr<VcsClient> vcs);

  /// \brief Adds a repository root.
  /// \param path The path to the repository root, relative to the filesystem's
  /// working directory.
  void AddRepositoryRoot(const llvm::Twine &path);

  /// \brief Exports a vnames.json file for all repositories.
  void ExportVNamesJson(const llvm::Twine &path);

  /// \brief Exports a script that, when run, will index all repositories.
  void ExportIndexScript(const llvm::Twine &path,
                         const llvm::Twine &vnames_json,
                         const llvm::Twine &subgraph);

  /// \brief Exports a subgraph describing all repositories.
  void ExportRepoSubgraph(const llvm::Twine &path);

  /// \brief An item on the worklist.
  struct WorkItem {
    /// The DFS depth to this WorkItem, used to prioritized path matches.
    size_t search_depth;
    /// The root path to inspect (about which we know little to nothing).
    std::string root_path;
  };

  /// \brief Change the default VCS ID.
  ///
  /// The default VCS ID is used instead of failing a repository if we can't
  /// determine a VCS ID from the VcsClient.
  void set_default_vcs_id(const std::string &new_id) {
    default_vcs_id_ = new_id;
  }

 private:
  /// \brief Add all repositories from the worklist until there aren't any left.
  void CrawlRepositories(std::vector<WorkItem> *worklist);
  /// \brief Writes a vnames.json-compatible JavaScript object to a Writer.
  /// \tparam W A RapidJson Writer.
  template <typename W>
  void WriteVNamesJson(W *writer);

  /// The filesystem we're reading from.
  llvm::IntrusiveRefCntPtr<clang::vfs::FileSystem> filesystem_;
  /// Repositories that we can extract and index.
  std::vector<std::unique_ptr<Repository>> repositories_;
  /// CommonJS registry to use for lookup.
  std::unique_ptr<CommonJsPackageRegistryClient> registry_;
  /// Version control system.
  std::unique_ptr<VcsClient> vcs_;
  /// Default version control system ID (if non-empty).
  std::string default_vcs_id_;
  /// Repository roots we've visited.
  std::set<std::string> visited_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_TOOLS_META_INDEX_META_INDEX_H_
