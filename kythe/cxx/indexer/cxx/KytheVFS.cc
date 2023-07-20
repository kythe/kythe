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

#include "KytheVFS.h"

#include <memory>
#include <system_error>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace kythe {

static inline std::pair<uint64_t, uint64_t> PairFromUid(
    const llvm::sys::fs::UniqueID& uid) {
  return {uid.getDevice(), uid.getFile()};
}

std::optional<llvm::sys::path::Style>
IndexVFS::DetectStyleFromAbsoluteWorkingDirectory(const std::string& awd) {
  if (llvm::sys::path::is_absolute(awd, llvm::sys::path::Style::posix)) {
    return llvm::sys::path::Style::posix;
  } else if (llvm::sys::path::is_absolute(awd,
                                          llvm::sys::path::Style::windows)) {
    return llvm::sys::path::Style::windows;
  }
  absl::FPrintF(stderr, "warning: could not detect path style for %s\n", awd);
  return std::nullopt;
}

namespace {
/// \brief normalizes `path` to POSIX style.
std::string FixupPath(llvm::StringRef path, llvm::sys::path::Style style) {
  if (style == llvm::sys::path::Style::windows &&
      llvm::sys::path::is_absolute(path, style)) {
    return absl::StrCat("/", path.str());
  }
  return std::string(path);
}
}  // anonymous namespace

IndexVFS::IndexVFS(absl::string_view working_directory,
                   const std::vector<proto::FileData>& virtual_files,
                   const std::vector<llvm::StringRef>& virtual_dirs,
                   llvm::sys::path::Style style)
    : working_directory_(FixupPath(
          llvm::StringRef(working_directory.data(), working_directory.size()),
          style)) {
  if (!llvm::sys::path::is_absolute(working_directory_,
                                    llvm::sys::path::Style::posix)) {
    LOG(WARNING) << "working directory " << working_directory_
                 << " is not absolute";
    // Various traversal routines assume that the working directory is absolute
    // and will recurse infinitely if it is not, so ensure that we follow this
    // invariant.
    working_directory_ = absl::StrCat("/", working_directory_);
  }
  for (const auto& data : virtual_files) {
    std::string path = FixupPath(data.info().path(), style);
    if (auto* record = FileRecordForPath(path, BehaviorOnMissing::kCreateFile,
                                         data.content().size())) {
      record->data =
          llvm::StringRef(data.content().data(), data.content().size());
    }
  }
  for (llvm::StringRef dir : virtual_dirs) {
    FileRecordForPath(FixupPath(dir, style),
                      BehaviorOnMissing::kCreateDirectory, 0);
  }
  // Clang always expects to be able to find a directory at .
  FileRecordForPath(".", BehaviorOnMissing::kCreateDirectory, 0);
}

llvm::ErrorOr<llvm::vfs::Status> IndexVFS::status(const llvm::Twine& path) {
  if (const auto* record =
          FileRecordForPath(path.str(), BehaviorOnMissing::kReturnError, 0)) {
    return record->status;
  }
  return make_error_code(llvm::errc::no_such_file_or_directory);
}

bool IndexVFS::get_vname(llvm::StringRef path, proto::VName* merge_with) {
  if (FileRecord* record =
          FileRecordForPath(path, BehaviorOnMissing::kReturnError, 0)) {
    if (record->status.getType() == llvm::sys::fs::file_type::regular_file &&
        record->has_vname) {
      *merge_with = record->vname;
      return true;
    }
  }
  return false;
}

llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>> IndexVFS::openFileForRead(
    const llvm::Twine& path) {
  if (FileRecord* record =
          FileRecordForPath(path.str(), BehaviorOnMissing::kReturnError, 0)) {
    if (record->status.getType() == llvm::sys::fs::file_type::regular_file) {
      return std::make_unique<File>(record);
    }
  }
  return make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::vfs::directory_iterator IndexVFS::dir_begin(const llvm::Twine& dir,
                                                  std::error_code& error_code) {
  std::string path = dir.str();
  if (auto* record =
          FileRecordForPath(path, BehaviorOnMissing::kReturnError, 0)) {
    return llvm::vfs::directory_iterator(
        std::make_shared<DirectoryIteratorImpl>(std::move(path), record));
  }
  error_code = std::make_error_code(std::errc::no_such_file_or_directory);
  return llvm::vfs::directory_iterator();
}

void IndexVFS::SetVName(const std::string& path, const proto::VName& vname) {
  if (FileRecord* record =
          FileRecordForPath(path, BehaviorOnMissing::kReturnError, 0)) {
    if (record->status.getType() == llvm::sys::fs::file_type::regular_file) {
      record->vname.CopyFrom(vname);
      record->has_vname = true;
    }
  }
}

bool IndexVFS::get_vname(const clang::FileEntry* entry,
                         proto::VName* merge_with) {
  auto record = uid_to_record_map_.find(PairFromUid(entry->getUniqueID()));
  if (record != uid_to_record_map_.end()) {
    if (record->second->status.getType() ==
            llvm::sys::fs::file_type::regular_file &&
        record->second->has_vname) {
      merge_with->CopyFrom(record->second->vname);
      return true;
    }
  }
  return false;
}

std::string IndexVFS::get_debug_uid_string(const llvm::sys::fs::UniqueID& uid) {
  auto record = uid_to_record_map_.find(PairFromUid(uid));
  if (record != uid_to_record_map_.end()) {
    return std::string(record->second->status.getName());
  }
  return "uid(device: " + std::to_string(uid.getDevice()) +
         " file: " + std::to_string(uid.getFile()) + ")";
}

IndexVFS::FileRecord* IndexVFS::FileRecordForPathRoot(const llvm::Twine& path,
                                                      bool create_if_missing) {
  std::string path_str(path.str());
  bool is_absolute = true;
  auto root_name =
      llvm::sys::path::root_name(path_str, llvm::sys::path::Style::posix);
  if (root_name.empty()) {
    root_name = llvm::sys::path::root_name(working_directory_,
                                           llvm::sys::path::Style::posix);
    if (!root_name.empty()) {
      // This index comes from a filesystem with significant root names.
      is_absolute = false;
    }
  }
  auto root_dir =
      llvm::sys::path::root_directory(path_str, llvm::sys::path::Style::posix);
  if (root_dir.empty()) {
    root_dir = llvm::sys::path::root_directory(working_directory_,
                                               llvm::sys::path::Style::posix);
    is_absolute = false;
  }
  if (!is_absolute) {
    // This terminates: the working directory must be an absolute path.
    return FileRecordForPath(working_directory_,
                             create_if_missing
                                 ? BehaviorOnMissing::kCreateDirectory
                                 : BehaviorOnMissing::kReturnError,
                             0);
  }
  FileRecord* name_record = nullptr;
  auto name_found = root_name_to_root_map_.find(std::string(root_name));
  if (name_found != root_name_to_root_map_.end()) {
    name_record = name_found->second;
  } else if (!create_if_missing) {
    return nullptr;
  } else {
    FileRecord record = {
        .status = llvm::vfs::Status(
            root_name, llvm::vfs::getNextVirtualUniqueID(),
            llvm::sys::TimePoint<>(), 0, 0, 0,
            llvm::sys::fs::file_type::directory_file, llvm::sys::fs::all_read),
        .has_vname = false,
        .label = std::string(root_name),
    };
    auto [iter, inserted] = uid_to_record_map_.insert_or_assign(
        PairFromUid(record.status.getUniqueID()),
        std::make_unique<FileRecord>(record));
    CHECK(inserted) << "Duplicated entries detected!";
    root_name_to_root_map_.insert_or_assign(std::string(root_name),
                                            iter->second.get());
    name_record = iter->second.get();
  }
  return AllocOrReturnFileRecord(name_record, create_if_missing, root_dir,
                                 llvm::sys::fs::file_type::directory_file, 0);
}

IndexVFS::FileRecord* IndexVFS::FileRecordForPath(llvm::StringRef path,
                                                  BehaviorOnMissing behavior,
                                                  size_t size) {
  using namespace llvm::sys::path;
  std::vector<llvm::StringRef> path_components;
  int skip_count = 0;

  auto eventual_type = (behavior == BehaviorOnMissing::kCreateFile
                            ? llvm::sys::fs::file_type::regular_file
                            : llvm::sys::fs::file_type::directory_file);
  bool create_if_missing = (behavior != BehaviorOnMissing::kReturnError);
  size_t eventual_size =
      (behavior == BehaviorOnMissing::kCreateFile ? size : 0);

  llvm::SmallString<1024> path_storage;
  if (llvm::sys::path::is_relative(path)) {
    llvm::sys::path::append(
        path_storage, llvm::sys::path::Style::posix,
        llvm::StringRef(working_directory_.data(), working_directory_.size()),
        path);
    path = llvm::StringRef(path_storage);
  }

  for (auto node = llvm::sys::path::rbegin(path, llvm::sys::path::Style::posix),
            node_end = rend(path);
       node != node_end; ++node) {
    if (*node == "..") {
      ++skip_count;
    } else if (*node != ".") {
      if (skip_count > 0) {
        --skip_count;
      } else {
        path_components.push_back(*node);
      }
    }
  }
  FileRecord* current_record = FileRecordForPathRoot(path, create_if_missing);
  for (auto node = path_components.crbegin(),
            node_end = path_components.crend();
       current_record != nullptr && node != node_end;) {
    llvm::StringRef label = *node;
    bool is_last = (++node == node_end);
    current_record = AllocOrReturnFileRecord(
        current_record, create_if_missing, label,
        is_last ? eventual_type : llvm::sys::fs::file_type::directory_file,
        is_last ? eventual_size : 0);
  }
  return current_record;
}

static const char* NameOfFileType(const llvm::sys::fs::file_type type) {
  switch (type) {
    case llvm::sys::fs::file_type::status_error:
      return "status_error";
    case llvm::sys::fs::file_type::file_not_found:
      return "file_not_found";
    case llvm::sys::fs::file_type::regular_file:
      return "regular_file";
    case llvm::sys::fs::file_type::directory_file:
      return "directory_file";
    case llvm::sys::fs::file_type::symlink_file:
      return "symlink_file";
    case llvm::sys::fs::file_type::block_file:
      return "block_file";
    case llvm::sys::fs::file_type::character_file:
      return "character_file";
    case llvm::sys::fs::file_type::fifo_file:
      return "fifo_file";
    case llvm::sys::fs::file_type::socket_file:
      return "socket_file";
    case llvm::sys::fs::file_type::type_unknown:
      return "type_unknown";
  }
}

IndexVFS::FileRecord* IndexVFS::AllocOrReturnFileRecord(
    FileRecord* parent, bool create_if_missing, llvm::StringRef label,
    llvm::sys::fs::file_type type, size_t size) {
  assert(parent != nullptr);
  for (auto& record : parent->children) {
    if (record->label == label) {
      if (create_if_missing && (record->status.getSize() != size ||
                                record->status.getType() != type)) {
        absl::FPrintF(
            stderr,
            "Warning: path %s/%s: defined inconsistently (%s:%d/%s:%d)\n",
            parent->status.getName().str(), label.str(), NameOfFileType(type),
            size, NameOfFileType(record->status.getType()),
            record->status.getSize());
        return nullptr;
      }
      return record;
    }
  }
  if (!create_if_missing) {
    return nullptr;
  }
  llvm::SmallString<1024> out_path(llvm::StringRef(parent->status.getName()));
  llvm::sys::path::append(out_path, llvm::sys::path::Style::posix, label);
  FileRecord record = {
      .status = llvm::vfs::Status(out_path, llvm::vfs::getNextVirtualUniqueID(),
                                  llvm::sys::TimePoint<>(), 0, 0, size, type,
                                  llvm::sys::fs::all_read),
      .has_vname = false,
      .label = std::string(label),
  };
  auto [iter, inserted] = uid_to_record_map_.insert_or_assign(
      PairFromUid(record.status.getUniqueID()),
      std::make_unique<FileRecord>(record));
  CHECK(inserted) << "Duplicate entries detected!";
  parent->children.push_back(iter->second.get());
  return iter->second.get();
}

llvm::vfs::directory_entry IndexVFS::DirectoryIteratorImpl::GetEntry() const {
  if (curr_ == end_) {
    return {};
  }
  llvm::SmallString<1024> path = llvm::StringRef(path_);
  llvm::sys::path::append(path, llvm::sys::path::Style::posix, (*curr_)->label);
  return {std::string(path), (*curr_)->status.getType()};
}

}  // namespace kythe
