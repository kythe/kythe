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

#ifndef KYTHE_CXX_COMMON_INDEXING_KYTHE_VFS_H_
#define KYTHE_CXX_COMMON_INDEXING_KYTHE_VFS_H_

#include "clang/Basic/FileManager.h"
#include "clang/Basic/VirtualFileSystem.h"
#include "kythe/proto/analysis.pb.h"

namespace kythe {

/// \brief A filesystem that allows access only to mapped files.
///
/// IndexVFS normalizes all paths (using the working directory for
/// relative paths). This means that foo/bar/../baz is assumed to be the
/// same as foo/baz.
class IndexVFS : public clang::vfs::FileSystem {
 public:
  /// \param working_directory The absolute path to the working directory.
  /// \param virtual_files Files to map.
  IndexVFS(const std::string &working_directory,
           const std::vector<proto::FileData> &virtual_files,
           const std::vector<llvm::StringRef> &virtual_dirs);
  ~IndexVFS();
  /// \brief Implements clang::vfs::FileSystem::status.
  llvm::ErrorOr<clang::vfs::Status> status(const llvm::Twine &path) override;
  /// \brief Implements clang::vfs::FileSystem::openFileForRead.
  llvm::ErrorOr<std::unique_ptr<clang::vfs::File>> openFileForRead(
      const llvm::Twine &path) override;
  /// \brief Unimplemented and unused.
  clang::vfs::directory_iterator dir_begin(
      const llvm::Twine &dir, std::error_code &error_code) override;
  /// \brief Associates a vname with a path.
  void SetVName(const std::string &path, const proto::VName &vname);
  /// \brief Returns the vname associated with some `FileEntry`.
  /// \param entry The `FileEntry` to look up.
  /// \param merge_with The `VName` to copy the vname onto.
  /// \return true if a match was found; false otherwise.
  bool get_vname(const clang::FileEntry *entry, proto::VName *merge_with);
  /// \brief Returns a string representation of `uid` for error messages.
  std::string get_debug_uid_string(const llvm::sys::fs::UniqueID &uid);
  const std::string &working_directory() const { return working_directory_; }
  llvm::ErrorOr<std::string> getCurrentWorkingDirectory() const override {
    return working_directory_;
  }
  std::error_code setCurrentWorkingDirectory(const llvm::Twine &Path) override {
    working_directory_ = Path.str();
    return std::error_code();
  }

 private:
  /// \brief Information kept on a file being tracked.
  struct FileRecord {
    /// Clang's VFS status record.
    clang::vfs::Status status;
    /// Whether `vname` is valid.
    bool has_vname;
    /// This file's name, independent of path.
    std::string label;
    /// This file's VName, if set.
    proto::VName vname;
    /// This directory's children.
    std::vector<FileRecord *> children;
    /// This file's content.
    llvm::StringRef data;
  };

  /// \brief A clang::vfs::File that wraps a `FileRecord`.
  class File : public clang::vfs::File {
   public:
    explicit File(FileRecord *record) : record_(record) {}
    llvm::ErrorOr<clang::vfs::Status> status() override {
      return record_->status;
    }
    std::error_code close() override { return std::error_code(); }
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> getBuffer(
        const llvm::Twine &Name, int64_t FileSize, bool RequiresNullTerminator,
        bool IsVolatile) override {
      name_ = Name.str();
      return llvm::MemoryBuffer::getMemBuffer(record_->data, name_,
                                              RequiresNullTerminator);
    }

   private:
    FileRecord *record_;
    std::string name_;
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
  FileRecord *FileRecordForPathRoot(const llvm::Twine &path,
                                    bool create_if_missing);

  /// \param path The path to investigate.
  /// \param behavior What to do if `path` does not exist.
  /// \param size The size of the file to use if kCreateFile is relevant.
  /// \return A `FileRecord` or nullptr on abort.
  FileRecord *FileRecordForPath(llvm::StringRef path,
                                BehaviorOnMissing behavior, size_t size);

  /// \brief Creates a new or returns an existing `FileRecord`.
  /// \param parent The parent `FileRecord`.
  /// \param create_if_missing Create a FileRecord if it's missing.
  /// \param label The label to look for under `parent`.
  /// \param type The type the record should have.
  /// \param size The size that should be used if this is a file record.
  FileRecord *AllocOrReturnFileRecord(FileRecord *parent,
                                      bool create_if_missing,
                                      llvm::StringRef label,
                                      llvm::sys::fs::file_type type,
                                      size_t size);

  /// The virtual files that were included in the index.
  const std::vector<proto::FileData> &virtual_files_;
  /// The working directory. Must be absolute.
  std::string working_directory_;
  /// Maps root names to root nodes. For indexes captured from Unix
  /// environments, there will be only one root name (the empty string).
  std::map<std::string, FileRecord *> root_name_to_root_map_;
  /// Maps unique IDs to file records.
  std::map<std::pair<uint64_t, uint64_t>, FileRecord *> uid_to_record_map_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEXING_KYTHE_VFS_H_
