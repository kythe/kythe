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
  /// \return nullopt if `awd` is not absolute or its style could not be
  /// detected; otherwise, the style of `awd`.
  static std::optional<llvm::sys::path::Style>
  DetectStyleFromAbsoluteWorkingDirectory(const std::string& awd);

  // IndexVFS is neither copyable nor movable.
  IndexVFS(const IndexVFS&) = delete;
  IndexVFS& operator=(const IndexVFS&) = delete;

  /// \brief Implements llvm::vfs::FileSystem::status.
  llvm::ErrorOr<llvm::vfs::Status> status(const llvm::Twine& path) override;
  /// \brief Implements llvm::vfs::FileSystem::openFileForRead.
  llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>> openFileForRead(
      const llvm::Twine& path) override;
  /// \brief Unimplemented and unused.
  llvm::vfs::directory_iterator dir_begin(const llvm::Twine& dir,
                                          std::error_code& error_code) override;
  /// \brief Associates a vname with a path.
  void SetVName(const std::string& path, const proto::VName& vname);
  /// \brief Returns the vname associated with some `FileEntry`.
  /// \param entry The `FileEntry` to look up.
  /// \param merge_with The `VName` to copy the vname onto.
  /// \return true if a match was found; false otherwise.
  bool get_vname(const clang::FileEntry* entry, proto::VName* merge_with);
  /// \brief Returns the vname associated with some `path`.
  /// \param path The path to look up.
  /// \param merge_with The `VName` to copy the vname onto.
  /// \return true if a match was found; false otherwise.
  bool get_vname(llvm::StringRef path, proto::VName* merge_with);

  /// \brief Returns a string representation of `uid` for error messages.
  std::string get_debug_uid_string(const llvm::sys::fs::UniqueID& uid);
  const std::string& working_directory() const { return working_directory_; }
  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override {
    return working_directory_;
  }
  std::error_code setCurrentWorkingDirectory(const llvm::Twine& Path) override {
    working_directory_ = Path.str();
    return std::error_code();
  }

 private:
  /// \brief Information kept on a file being tracked.
  struct FileRecord {
    /// Clang's VFS status record.
    llvm::vfs::Status status;
    /// Whether `vname` is valid.
    bool has_vname;
    /// This file's name, independent of path.
    std::string label;
    /// This file's VName, if set.
    proto::VName vname;
    /// This directory's children.
    std::vector<FileRecord*> children;
    /// This file's content.
    llvm::StringRef data;
  };

  /// \brief A llvm::vfs::File that wraps a `FileRecord`.
  class File : public llvm::vfs::File {
   public:
    explicit File(FileRecord* record) : record_(record) {}
    llvm::ErrorOr<llvm::vfs::Status> status() override {
      return record_->status;
    }
    std::error_code close() override { return std::error_code(); }
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> getBuffer(
        const llvm::Twine& Name, int64_t FileSize, bool RequiresNullTerminator,
        bool IsVolatile) override {
      name_ = Name.str();
      return llvm::MemoryBuffer::getMemBuffer(record_->data, name_,
                                              RequiresNullTerminator);
    }

   private:
    FileRecord* record_;
    std::string name_;
  };

  class DirectoryIteratorImpl : public ::llvm::vfs::detail::DirIterImpl {
   public:
    explicit DirectoryIteratorImpl(std::string path, const FileRecord* record)
        : path_(std::move(path)), record_(record) {
      CurrentEntry = GetEntry();
    }

    std::error_code increment() override {
      ++curr_;
      CurrentEntry = GetEntry();
      return {};
    }

   private:
    ::llvm::vfs::directory_entry GetEntry() const;

    std::string path_;
    const FileRecord* record_;
    std::vector<FileRecord*>::const_iterator curr_ = record_->children.begin(),
                                             end_ = record_->children.end();
  };

  /// \brief Controls what happens when a missing path node is encountered.
  enum class BehaviorOnMissing {
    kCreateFile,       ///< Create intermediate directories and a final file.
    kCreateDirectory,  ///< Create intermediate and final directories.
    kReturnError       ///< Abort.
  };

  /// \brief Returns a FileRecord for the root components of `path`.
  /// \param path The path to investigate.
  /// \param create_if_missing If the root is missing, create it.
  /// \return A `FileRecord` or nullptr on abort.
  FileRecord* FileRecordForPathRoot(const llvm::Twine& path,
                                    bool create_if_missing);

  /// \param path The path to investigate.
  /// \param behavior What to do if `path` does not exist.
  /// \param size The size of the file to use if kCreateFile is relevant.
  /// \return A `FileRecord` or nullptr on abort.
  FileRecord* FileRecordForPath(llvm::StringRef path,
                                BehaviorOnMissing behavior, size_t size);

  /// \brief Creates a new or returns an existing `FileRecord`.
  /// \param parent The parent `FileRecord`.
  /// \param create_if_missing Create a FileRecord if it's missing.
  /// \param label The label to look for under `parent`.
  /// \param type The type the record should have.
  /// \param size The size that should be used if this is a file record.
  FileRecord* AllocOrReturnFileRecord(FileRecord* parent,
                                      bool create_if_missing,
                                      llvm::StringRef label,
                                      llvm::sys::fs::file_type type,
                                      size_t size);

  /// The working directory. Must be absolute.
  std::string working_directory_;
  /// Maps root names to root nodes. For indexes captured from Unix
  /// environments, there will be only one root name (the empty string).
  absl::flat_hash_map<std::string, FileRecord*> root_name_to_root_map_;
  /// Maps unique IDs to file records.
  absl::flat_hash_map<std::pair<uint64_t, uint64_t>,
                      std::unique_ptr<FileRecord>>
      uid_to_record_map_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEXING_KYTHE_VFS_H_
