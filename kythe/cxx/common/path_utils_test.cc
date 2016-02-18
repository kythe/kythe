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

#include "path_utils.h"

#include <string>

#include "clang/Tooling/Tooling.h"
#include "gtest/gtest.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

namespace kythe {
namespace {

TEST(PathUtilsTest, RelativizePath) {
  llvm::SmallString<128> current_dir_llvm;
  std::error_code get_dir_failed =
      llvm::sys::fs::current_path(current_dir_llvm);
  ASSERT_FALSE(get_dir_failed);

  llvm::SmallString<128> cwd_foo_llvm = current_dir_llvm;
  llvm::sys::path::append(cwd_foo_llvm, "foo");

  std::string current_dir(current_dir_llvm.str());
  std::string cwd_foo(cwd_foo_llvm.str());

  EXPECT_EQ("foo", RelativizePath("foo", "."));
  EXPECT_EQ("foo", RelativizePath("foo", current_dir));
  EXPECT_EQ("bar", RelativizePath("foo/bar", "foo"));
  EXPECT_EQ("bar", RelativizePath("foo/bar", cwd_foo));
  EXPECT_EQ("foo", RelativizePath(cwd_foo, "."));
  EXPECT_EQ(cwd_foo, RelativizePath(cwd_foo, "bar"));
}

TEST(PathUtilsTest, MakeCleanAbsolutePath) {
  llvm::SmallString<128> current_dir_llvm;
  std::error_code get_dir_failed =
      llvm::sys::fs::current_path(current_dir_llvm);
  ASSERT_FALSE(get_dir_failed);
  std::string current_dir(current_dir_llvm.str());
  EXPECT_EQ("/a/b/c", MakeCleanAbsolutePath("/a/b/c"));
  EXPECT_EQ("/a/b/c", MakeCleanAbsolutePath("/a/b/c/."));
  EXPECT_EQ("/a/b", MakeCleanAbsolutePath("/a/b/c/./.."));
  EXPECT_EQ("/a/b", MakeCleanAbsolutePath("/a/b/c/../."));
  EXPECT_EQ("/a/b", MakeCleanAbsolutePath("/a/b/c/.."));
  EXPECT_EQ("/", MakeCleanAbsolutePath("/a/../c/.."));
  EXPECT_EQ("/", MakeCleanAbsolutePath("/a/b/c/../../.."));
  EXPECT_EQ(current_dir, MakeCleanAbsolutePath("."));
}

TEST(PathUtilsTest, CleanPath) {
  EXPECT_EQ("/a/c", CleanPath("/../../a/c"));
  EXPECT_EQ("", CleanPath(""));
  // CleanPath should match the semantics of Go's path.Clean (except for
  // "" => "", not "" => "."); the examples from the documentation at
  // http://golang.org/pkg/path/#example_Clean are checked here.
  EXPECT_EQ("a/c", CleanPath("a/c"));
  EXPECT_EQ("a/c", CleanPath("a//c"));
  EXPECT_EQ("a/c", CleanPath("a/c/."));
  EXPECT_EQ("a/c", CleanPath("a/c/b/.."));
  EXPECT_EQ("/a/c", CleanPath("/../a/c"));
  EXPECT_EQ("/a/c", CleanPath("/../a/b/../././/c"));
  EXPECT_EQ("/Users", CleanPath("/Users"));
  // "//Users" denotes a path with the root name "//Users"
  EXPECT_EQ("//Users", CleanPath("//Users"));
  EXPECT_EQ("/Users", CleanPath("///Users"));
}

TEST(PathUtilsTest, JoinPath) {
  EXPECT_EQ("a/c", JoinPath("a", "c"));
  EXPECT_EQ("a/c", JoinPath("a/", "c"));
  EXPECT_EQ("a/c", JoinPath("a", "/c"));
}

}  // anonymous namespace
}  // namespace kythe

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
