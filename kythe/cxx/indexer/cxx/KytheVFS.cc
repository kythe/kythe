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

#include "KytheVFS.h"

#include "kythe/cxx/common/proto_conversions.h"

#include "llvm/Support/Path.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"

namespace kythe {

IndexVFS::IndexVFS(const std::string &working_directory,
                   const std::vector<proto::FileData> &virtual_files)
    : virtual_files_(virtual_files), working_directory_(working_directory) {
  assert(llvm::sys::path::is_absolute(working_directory) &&
         "Working directory must be absolute.");
  for (const auto &data : virtual_files_) {
    if (auto *record = FileRecordForPath(ToStringRef(data.info().path()),
                                         BehaviorOnMissing::kCreateFile,
                                         data.content().size())) {
      record->data = llvm::StringRef(data.content());
    }
  }
}

IndexVFS::~IndexVFS() {
  for (auto &entry : uid_to_record_map_) {
    delete entry.second;
  }
}

llvm::ErrorOr<clang::vfs::Status> IndexVFS::status(const llvm::Twine &path) {
  if (const auto *record =
          FileRecordForPath(path.str(), BehaviorOnMissing::kReturnError, 0)) {
    return record->status;
  }
  return make_error_code(llvm::errc::no_such_file_or_directory);
}

llvm::ErrorOr<std::unique_ptr<clang::vfs::File>> IndexVFS::openFileForRead(
    const llvm::Twine &path) {
  if (FileRecord *record =
          FileRecordForPath(path.str(), BehaviorOnMissing::kReturnError, 0)) {
    if (record->status.getType() == llvm::sys::fs::file_type::regular_file) {
      return std::unique_ptr<clang::vfs::File>(new File(record));
    }
  }
  return make_error_code(llvm::errc::no_such_file_or_directory);
}

clang::vfs::directory_iterator IndexVFS::dir_begin(
    const llvm::Twine &dir, std::error_code &error_code) {
  llvm_unreachable("unimplemented");
}

void IndexVFS::SetVName(const std::string &path, const proto::VName &vname) {
  if (FileRecord *record =
          FileRecordForPath(path, BehaviorOnMissing::kReturnError, 0)) {
    if (record->status.getType() == llvm::sys::fs::file_type::regular_file) {
      record->vname.CopyFrom(vname);
      record->has_vname = true;
    }
  }
}

bool IndexVFS::get_vname(const clang::FileEntry *entry,
                         proto::VName *merge_with) {
  auto record = uid_to_record_map_.find(entry->getUniqueID());
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

std::string IndexVFS::get_debug_uid_string(const llvm::sys::fs::UniqueID &uid) {
  auto record = uid_to_record_map_.find(uid);
  if (record != uid_to_record_map_.end()) {
    return record->second->status.getName();
  }
  return "uid(device: " + std::to_string(uid.getDevice()) + " file: " +
         std::to_string(uid.getFile()) + ")";
}

IndexVFS::FileRecord *IndexVFS::FileRecordForPathRoot(const llvm::Twine &path,
                                                      bool create_if_missing) {
  std::string path_str(path.str());
  bool is_absolute = true;
  auto root_name = llvm::sys::path::root_name(path_str);
  if (root_name.empty()) {
    root_name = llvm::sys::path::root_name(working_directory_);
    if (!root_name.empty()) {
      // This index comes from a filesystem with significant root names.
      is_absolute = false;
    }
  }
  auto root_dir = llvm::sys::path::root_directory(path_str);
  if (root_dir.empty()) {
    root_dir = llvm::sys::path::root_directory(working_directory_);
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
  FileRecord *name_record = nullptr;
  auto name_found = root_name_to_root_map_.find(root_name);
  if (name_found != root_name_to_root_map_.end()) {
    name_record = name_found->second;
  } else if (!create_if_missing) {
    return nullptr;
  } else {
    name_record = new FileRecord(
        {clang::vfs::Status(
             root_name, root_name, clang::vfs::getNextVirtualUniqueID(),
             llvm::sys::TimeValue(), 0, 0, 0,
             llvm::sys::fs::file_type::directory_file, llvm::sys::fs::all_read),
         false, root_name});
    root_name_to_root_map_[root_name] = name_record;
    uid_to_record_map_[name_record->status.getUniqueID()] = name_record;
  }
  return AllocOrReturnFileRecord(name_record, create_if_missing, root_dir,
                                 llvm::sys::fs::file_type::directory_file, 0);
}

IndexVFS::FileRecord *IndexVFS::FileRecordForPath(const llvm::StringRef path,
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

  for (auto node = llvm::sys::path::rbegin(path), node_end = rend(path);
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
  FileRecord *current_record = FileRecordForPathRoot(path, create_if_missing);
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

IndexVFS::FileRecord *IndexVFS::AllocOrReturnFileRecord(
    FileRecord *parent, bool create_if_missing, llvm::StringRef label,
    llvm::sys::fs::file_type type, size_t size) {
  assert(parent != nullptr);
  for (auto &record : parent->children) {
    if (record->label == label) {
      if (create_if_missing && (record->status.getSize() != size ||
                                record->status.getType() != type)) {
        fprintf(stderr, "Warning: path %s/%s: defined inconsistently (%d/%d)\n",
                parent->status.getName().str().c_str(), label.str().c_str(),
                type, record->status.getType());
        return nullptr;
      }
      return record;
    }
  }
  if (!create_if_missing) {
    return nullptr;
  }
  llvm::SmallString<1024> out_path(llvm::StringRef(parent->status.getName()));
  llvm::sys::path::append(out_path, label);
  FileRecord *new_record = new FileRecord{
      clang::vfs::Status(
          out_path, out_path, clang::vfs::getNextVirtualUniqueID(),
          llvm::sys::TimeValue(), 0, 0, size, type, llvm::sys::fs::all_read),
      false, label};
  parent->children.push_back(new_record);
  uid_to_record_map_[new_record->status.getUniqueID()] = new_record;
  return new_record;
}

}  // namespace kythe
