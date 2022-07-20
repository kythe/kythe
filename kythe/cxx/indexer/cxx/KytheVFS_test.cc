/*
 * Copyright 2022 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/indexer/cxx/KytheVFS.h"

#include <initializer_list>
#include <string>
#include <system_error>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "kythe/proto/analysis.pb.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/VirtualFileSystem.h"

namespace kythe {
namespace {
using ::testing::Optional;
using ::testing::UnorderedElementsAre;

template <typename Container = std::initializer_list<std::string>>
std::vector<proto::FileData> FileDataFromPaths(Container&& paths) {
  std::vector<proto::FileData> result;
  for (const auto& path : paths) {
    result.emplace_back().mutable_info()->set_path(path);
  }
  return result;
}

TEST(KytheVFSTest, AddingFilesCreatesTraversableTree) {
  using ::llvm::vfs::recursive_directory_iterator;

  auto files = FileDataFromPaths({
      "path/to/a/file.txt",
      "path/to/another/file.txt",
  });
  IndexVFS vfs("/working_directory", files, {}, llvm::sys::path::Style::posix);

  std::vector<std::string> results;
  std::error_code err;
  for (recursive_directory_iterator iter(vfs, "/", err), end;
       iter != end && !err; iter.increment(err)) {
    results.push_back(iter->path().str());
  }
  ASSERT_FALSE(err) << err;
  EXPECT_THAT(results,
              UnorderedElementsAre(
                  "/working_directory", "/working_directory/path",
                  "/working_directory/path/to", "/working_directory/path/to/a",
                  "/working_directory/path/to/a/file.txt",
                  "/working_directory/path/to/another",
                  "/working_directory/path/to/another/file.txt"));
}

TEST(KytheVFSTest, AddingFilesCreatesParentDirectories) {
  using ::llvm::sys::fs::file_type;

  auto files = FileDataFromPaths({
      "path/to/a/file.txt",
      "path/to/another/file.txt",
  });
  IndexVFS vfs("/working_directory", files, {}, llvm::sys::path::Style::posix);
  struct Entry {
    std::string path;
    file_type type = file_type::directory_file;
  };
  std::vector<Entry> expected = {
      {"/"},
      {"/working_directory"},
      {"/working_directory/path"},
      {"/working_directory/path/to"},
      {"/working_directory/path/to/a"},
      {"/working_directory/path/to/a/file.txt", file_type::regular_file},
      {"/working_directory/path/to/another"},
      {"/working_directory/path/to/another/file.txt", file_type::regular_file},
  };

  for (const auto& entry : expected) {
    EXPECT_THAT(vfs.status(entry.path).get().getType(), entry.type)
        << "Missing expected path: " << entry.path;
  }
}

TEST(KytheVFSTest, RelativeFilesCreatesParentDirectories) {
  using ::llvm::sys::fs::file_type;

  auto files = FileDataFromPaths({
      "../path/to/a/file.txt",
      "../path/to/another/file.txt",
  });
  IndexVFS vfs("/working_directory", files, {}, llvm::sys::path::Style::posix);
  struct Entry {
    std::string path;
    file_type type = file_type::directory_file;
  };
  std::vector<Entry> expected = {
      {"/"},
      {"/working_directory"},
      {"/path"},
      {"/path"},
      {"/path/to"},
      {"/path/to/a"},
      {"/path/to/a/file.txt", file_type::regular_file},
      {"/path/to/another"},
      {"/path/to/another/file.txt", file_type::regular_file},
  };

  for (const auto& entry : expected) {
    EXPECT_THAT(vfs.status(entry.path).get().getType(), entry.type)
        << "Missing expected path: " << entry.path;
  }
}

TEST(KytheVFSTest, DetectStyleFromAbsoluteWorkingDirectoryDoes) {
  EXPECT_THAT(
      IndexVFS::DetectStyleFromAbsoluteWorkingDirectory("/posix/style/path"),
      Optional(llvm::sys::path::Style::posix));
  EXPECT_THAT(IndexVFS::DetectStyleFromAbsoluteWorkingDirectory(
                  "C:/funky/windows/path"),
              Optional(llvm::sys::path::Style::windows));
}

}  // namespace
}  // namespace kythe
