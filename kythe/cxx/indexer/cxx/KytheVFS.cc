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
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "kythe/cxx/indexer/cxx/stream_adapter.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"

namespace kythe {
namespace {

using PathString = ::llvm::SmallString<256>;
using ::llvm::sys::path::is_absolute;
using ::llvm::sys::path::Style;

}  // namespace

std::optional<llvm::sys::path::Style>
IndexVFS::DetectStyleFromAbsoluteWorkingDirectory(const std::string& awd) {
  if (is_absolute(awd, Style::posix)) {
    return Style::posix;
  } else if (is_absolute(awd, Style::windows)) {
    return Style::windows;
  }
  absl::FPrintF(stderr, "warning: could not detect path style for %s\n", awd);
  return std::nullopt;
}

IndexVFS::RootDirectory::RootDirectory(const llvm::Twine& path, Style style) {
  PathString preferred;
  path.toVector(preferred);
  llvm::sys::path::make_preferred(preferred, style);
  if (!llvm::sys::path::is_absolute(preferred, Style::posix)) {
    LOG(WARNING) << "working directory " << StreamAdapter::Stream(preferred)
                 << " is not absolute";
    preferred.insert(preferred.begin(), '/');
  }
  value_ = std::string{preferred};
}

IndexVFS::RootStyle IndexVFS::DetectRootStyle(
    const llvm::Twine& working_directory) {
  if (is_absolute(working_directory, Style::posix)) {
    return RootStyle{
        .root = RootDirectory{working_directory, Style::posix},
        .style = Style::posix,
    };
  }

  Style style = Style::posix;
  PathString storage;
  llvm::StringRef path = working_directory.toStringRef(storage);
  if (const auto n = path.find_first_of("/\\"); n != path.npos) {
    style = (path[n] == '/') ? Style::windows_slash : Style::windows_backslash;
    if (style == Style::windows_slash &&
        !is_absolute(working_directory, style)) {
      // If it's not an absolute Windows-style path, assume POSIX and allow
      // RootDirectory to make it absolute.
      style = Style::posix;
    }
  }
  return RootStyle{
      .root = RootDirectory{working_directory, style},
      .style = style,
  };
}

IndexVFS::IndexVFS(RootDirectory root) {
  setCurrentWorkingDirectory(root.value());
  // Ensure there's always an entry for "."
  AddDirectory(root.value());
}

IndexVFS::IndexVFS(absl::string_view working_directory,
                   const std::vector<proto::FileData>& virtual_files,
                   const std::vector<llvm::StringRef>& virtual_dirs,
                   llvm::sys::path::Style style)
    : IndexVFS(RootDirectory(working_directory, style)) {
  absl::flat_hash_map<absl::string_view, absl::string_view>
      canonical_digest_map;
  for (const auto& data : virtual_files) {
    auto [iter, inserted] = canonical_digest_map.try_emplace(
        data.info().digest(), data.info().path());
    PathString preferred(data.info().path());
    llvm::sys::path::make_preferred(preferred, style);
    if (inserted) {
      AddFile(preferred,
              llvm::MemoryBuffer::getMemBuffer(
                  data.content(), /* BufferName */ data.info().digest(),
                  /* RequiresNullTerminator */ false));
    } else {
      // Treat files with identical digests as "identical" files for UniqueID
      // purposes so that `#pragma once` works as expected for files with
      // multiple paths.
      PathString target(iter->second);
      llvm::sys::path::make_preferred(target, style);
      AddLink(preferred, target);
    }
  }

  for (llvm::StringRef dir : virtual_dirs) {
    PathString preferred(dir);
    llvm::sys::path::make_preferred(preferred, style);
    AddDirectory(dir);
  }
}

bool IndexVFS::AddDirectory(const llvm::Twine& path) {
  if (fs_.exists(path)) {
    // InMemoryFileSystem doesn't deal well with duplicate entries.
    return false;
  }
  fs_.addFile(path, /* ModificationTime */ {},
              /* Buffer */ nullptr,
              /* User */ std::nullopt, /* Group */ std::nullopt,
              llvm::sys::fs::file_type::directory_file);
  return true;
}

bool IndexVFS::AddFile(const llvm::Twine& path,
                       std::unique_ptr<llvm::MemoryBuffer> data) {
  if (fs_.exists(path)) {
    return false;
  }
  fs_.addFile(path, /* ModificationTime */ {}, /* Buffer */ std::move(data));
  if (auto status = fs_.status(path)) {
    // Use the first insertion for the canonical name.
    debug_uid_name_map_.try_emplace(status->getUniqueID(), status->getName());
  }
  return true;
}

bool IndexVFS::AddLink(const llvm::Twine& path, const llvm::Twine& target) {
  return fs_.addHardLink(path, target);
}

llvm::ErrorOr<llvm::vfs::Status> IndexVFS::status(const llvm::Twine& path) {
  return fs_.status(path);
}

llvm::ErrorOr<std::unique_ptr<llvm::vfs::File>> IndexVFS::openFileForRead(
    const llvm::Twine& path) {
  return fs_.openFileForRead(path);
}

llvm::vfs::directory_iterator IndexVFS::dir_begin(const llvm::Twine& dir,
                                                  std::error_code& error_code) {
  return fs_.dir_begin(dir, error_code);
}

bool IndexVFS::SetVName(const llvm::Twine& path, const proto::VName& vname) {
  PathString realized;
  if (fs_.getRealPath(path, realized)) {
    return false;
  }

  Entry* entry = nullptr;
  absl::flat_hash_map<std::string, Entry>* children = &roots_;
  for (auto iter = llvm::sys::path::begin(realized),
            end = llvm::sys::path::end(realized);
       iter != end; ++iter) {
    entry = &children->try_emplace(*iter).first->second;
    children = &entry->children;
  }
  if (entry == nullptr) {
    return false;
  }
  entry->vname = vname;
  return true;
}

bool IndexVFS::GetVName(clang::FileEntryRef entry, proto::VName& merge_with) {
  // VName's are mapped by path, but there may be multiple paths for the same
  // UniqueId. This can happen with symlinks/hardlinks. We need to have distinct
  // enrties for these to support `#pragma once` and accurately reflect builds
  // on CAS-backed filesystems.
  return GetVName(entry.getNameAsRequested(), merge_with);
}

proto::VName* IndexVFS::GetVName(const llvm::Twine& path) {
  PathString realized;
  if (fs_.getRealPath(path, realized)) {
    return nullptr;
  }

  Entry* entry = nullptr;
  absl::flat_hash_map<std::string, Entry>* children = &roots_;
  for (auto iter = llvm::sys::path::begin(realized),
            end = llvm::sys::path::end(realized);
       iter != end; ++iter) {
    auto child = children->find(*iter);
    if (child == children->end()) {
      return nullptr;
    }
    entry = &child->second;
    children = &entry->children;
  }
  if (entry == nullptr) {
    return nullptr;
  }
  return entry->vname.has_value() ? &*entry->vname : nullptr;
}

bool IndexVFS::GetVName(const llvm::Twine& path, proto::VName& result) {
  proto::VName* vname = GetVName(path);
  if (vname == nullptr) {
    return false;
  }
  result = *vname;
  return true;
}

std::string IndexVFS::get_debug_uid_string(const llvm::sys::fs::UniqueID& uid) {
  auto iter = debug_uid_name_map_.find(uid);
  if (iter != debug_uid_name_map_.end()) {
    return iter->second;
  }
  return absl::StrCat("uid(device: ", uid.getDevice(), " file: ", uid.getFile(),
                      ")");
}
}  // namespace kythe
