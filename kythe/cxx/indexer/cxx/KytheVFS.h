/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_INDEXING_KYTHE_VFS_H_
#define KYTHE_CXX_COMMON_INDEXING_KYTHE_VFS_H_

#include <memory>
#include <optional>

#include "absl/base/attributes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/hash/hash.h"
#include "absl/strings/string_view.h"
#include "clang/Basic/FileManager.h"
#include "kythe/proto/analysis.pb.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace kythe {

/// \brief A filesystem that allows access only to mapped files.
///
/// IndexVFS normalizes all paths (using the working directory for
/// relative paths). This means that foo/bar/../baz is assumed to be the
/// same as foo/baz.
class IndexVFS : public llvm::vfs::FileSystem {
 public:
  /// \brief An opaque type for an absolute, POSIX-style path.
  class RootDirectory {
   public:
    /// \brief Constructs a new RootDirectory for the given path.
    /// \param path The path to use. If not already aboslute, a `/` will be
    /// inserted making it so.
    /// \param style The style from which to convert the path.
    explicit RootDirectory(
        const llvm::Twine& path,
        llvm::sys::path::Style style = llvm::sys::path::Style::posix);

    /// \brief Returns the current path value.
    const std::string& value() const& { return value_; }
    std::string&& value() && { return std::move(value_); }

   private:
    friend bool operator==(const RootDirectory& lhs, const RootDirectory& rhs) {
      return lhs.value_ == rhs.value_;
    };
    friend bool operator==(const RootDirectory& lhs, llvm::StringRef rhs) {
      return lhs.value_ == rhs;
    };
    friend bool operator==(llvm::StringRef lhs, const RootDirectory& rhs) {
      return lhs == rhs.value_;
    };
    template <typename T, typename U>
    friend bool operator!=(const T& lhs, const T& rhs) {
      return !(lhs == rhs);
    }
    std::string value_;
  };

  /// \brief The RootDirectory and detected path style.
  struct RootStyle {
    RootDirectory root;
    llvm::sys::path::Style style;
  };

  /// \brief Detects the path style from `working_directory`, converts it to an
  /// absolute directory in the preferred style and returns the pair.
  static RootStyle DetectRootStyle(const llvm::Twine& working_directory);

  /// \brief Constructs a new IndexVFS from the given absolute root directory.
  explicit IndexVFS(RootDirectory root);

  /// \param working_directory The absolute path to the working directory.
  /// \param virtual_files Files to map.  File content must outlive IndexVFS.
  /// \param virtual_dirs Directories to map.
  /// \param style Style used to parse incoming paths. Paths are normalized
  /// to POSIX-style.
  explicit IndexVFS(absl::string_view working_directory,
                    const std::vector<proto::FileData>& virtual_files
                        ABSL_ATTRIBUTE_LIFETIME_BOUND,
                    const std::vector<llvm::StringRef>& virtual_dirs,
                    llvm::sys::path::Style style);
  explicit IndexVFS(RootDirectory root,
                    const std::vector<proto::FileData>& virtual_files
                        ABSL_ATTRIBUTE_LIFETIME_BOUND,
                    const std::vector<llvm::StringRef>& virtual_dirs,
                    llvm::sys::path::Style style);

  // IndexVFS is neither copyable nor movable.
  IndexVFS(const IndexVFS&) = delete;
  IndexVFS& operator=(const IndexVFS&) = delete;

  /// \brief Add the named directory to the file system, if not already present.
  bool AddDirectory(const llvm::Twine& path);
  /// \brief Add the named file to the file system, if not already present.
  bool AddFile(const llvm::Twine& path,
               std::unique_ptr<llvm::MemoryBuffer> data);
  /// \brief Add `path` as an alias of `target` which must already exist.
  bool AddLink(const llvm::Twine& path, const llvm::Twine& target);

  /// \brief Associates a vname with a path.
  bool SetVName(const llvm::Twine& path, const proto::VName& vname);
  /// \brief Returns the vname associated with some `FileEntry`.
  /// \param entry The `FileEntry` to look up.
  /// \param merge_with The `VName` to copy the vname onto.
  /// \return true if a match was found; false otherwise.
  bool GetVName(clang::FileEntryRef entry, proto::VName& merge_with);
  /// \brief Returns the vname associated with some `path`.
  /// \param path The path to look up.
  /// \param merge_with The `VName` to copy the vname onto.
  /// \return true if a match was found; false otherwise.
  bool GetVName(const llvm::Twine& path, proto::VName& result);
  const proto::VName* GetVName(const llvm::Twine& path);

  /// \brief Returns a normalized, canonical root-relative path.
  std::optional<std::string> GetRelativePath(const llvm::Twine& path);
  /// \brief Returns a normalized, canonical absolute path.
  std::optional<std::string> GetCanonicalPath(const llvm::Twine& path);

  /// \brief Returns a string representation of `uid` for error messages.
  std::string get_debug_uid_string(const llvm::sys::fs::UniqueID& uid);
  std::string working_directory() const {
    return *getCurrentWorkingDirectory();
  }

  /// \brief Implements llvm::vfs::FileSystem::status.
  llvm::ErrorOr<llvm::vfs::Status> status(const llvm::Twine& path) override;
  /// \brief Implements llvm::vfs::FileSystem::openFileForRead.
  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>> openFileForRead(
      const llvm::Twine& path) override;
  /// \brief Unimplemented and unused.
  llvm::vfs::directory_iterator dir_begin(const llvm::Twine& dir,
                                          std::error_code& error_code) override;

  /// \brief Implements FileSystem::getCurrentWorkingDirectory
  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override {
    return fs_.getCurrentWorkingDirectory();
  }
  /// \brief Implements FileSystem::setCurrentWorkingDirectory
  std::error_code setCurrentWorkingDirectory(const llvm::Twine& Path) override {
    return fs_.setCurrentWorkingDirectory(Path);
  }
  /// \brief Implements FileSystem::getRealPath
  std::error_code getRealPath(const llvm::Twine& Path,
                              llvm::SmallVectorImpl<char>& Output) override {
    return fs_.getRealPath(Path, Output);
  }

 private:
  struct Entry {
    std::optional<proto::VName> vname;
    absl::flat_hash_map<std::string, Entry> children;
  };

  // TODO(shahms): Remove these. This is only here to support
  // get_debug_uid_string used by KytheGraphObserver
  struct UniqueIDHasher {
    size_t operator()(const llvm::sys::fs::UniqueID& id) const {
      return absl::HashOf(id.getDevice(), id.getFile());
    }
  };
  absl::flat_hash_map<llvm::sys::fs::UniqueID, std::string, UniqueIDHasher>
      debug_uid_name_map_;

  absl::flat_hash_map<std::string, Entry> roots_;
  llvm::vfs::InMemoryFileSystem fs_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEXING_KYTHE_VFS_H_
