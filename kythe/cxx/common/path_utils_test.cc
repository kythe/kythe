/*
 * Copyright 2018 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/path_utils.h"

#include <unistd.h>

#include <string>
#include <system_error>

#include "gtest/gtest.h"
#include "kythe/cxx/common/status_or.h"

namespace kythe {
namespace {

std::error_code symlink(const char* target, const char* linkpath) {
  if (::symlink(target, linkpath) < 0) {
    return std::error_code(errno, std::generic_category());
  }
  return std::error_code();
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
  EXPECT_EQ("..", CleanPath("a/../../"));
}

TEST(PathUtilsTest, JoinPath) {
  EXPECT_EQ("a/c", JoinPath("a", "c"));
  EXPECT_EQ("a/c", JoinPath("a/", "c"));
  EXPECT_EQ("a/c", JoinPath("a", "/c"));
}

TEST(PathUtilsTest, RelativizePath) {
  StatusOr<std::string> current_dir = GetCurrentDirectory();
  ASSERT_TRUE(current_dir.ok());

  std::string cwd_foo = JoinPath(*current_dir, "foo");

  EXPECT_EQ("foo", RelativizePath("foo", "."));
  EXPECT_EQ("foo", RelativizePath("foo", *current_dir));
  EXPECT_EQ("bar", RelativizePath("foo/bar", "foo"));
  EXPECT_EQ("bar", RelativizePath("foo/bar", cwd_foo));
  EXPECT_EQ("foo", RelativizePath(cwd_foo, "."));
  EXPECT_EQ(cwd_foo, RelativizePath(cwd_foo, "bar"));

  // If all paths are absolute, then relativizing is unaffected by current_dir.
  EXPECT_EQ("bar", RelativizePath("/foo/bar", "/foo"));
  EXPECT_EQ("foo", RelativizePath("/foo", "/"));

  // Test that we only accept proper path prefixes as parent.
  EXPECT_EQ("/foooo/bar", RelativizePath("/foooo/bar", "/foo"));
}

TEST(PathUtilsTest, MakeCleanAbsolutePath) {
  std::string current_dir = GetCurrentDirectory().ValueOrDie();

  EXPECT_EQ(current_dir, MakeCleanAbsolutePath(".").ValueOrDie());

  EXPECT_EQ("/a/b/c", MakeCleanAbsolutePath("/a/b/c").ValueOrDie());
  EXPECT_EQ("/a/b/c", MakeCleanAbsolutePath("/a/b/c/.").ValueOrDie());
  EXPECT_EQ("/a/b", MakeCleanAbsolutePath("/a/b/c/./..").ValueOrDie());
  EXPECT_EQ("/a/b", MakeCleanAbsolutePath("/a/b/c/../.").ValueOrDie());
  EXPECT_EQ("/a/b", MakeCleanAbsolutePath("/a/b/c/..").ValueOrDie());
  EXPECT_EQ("/", MakeCleanAbsolutePath("/a/../c/..").ValueOrDie());
  EXPECT_EQ("/", MakeCleanAbsolutePath("/a/b/c/../../..").ValueOrDie());
}

TEST(PathUtilsTest, RealPath) {
  // Since RealPath accesses the filesystem, we can't make very many
  // guarantees about its behavior, but we would like it to return an
  // error if a path doesn't exist.
  StatusOr<std::string> result = RealPath("/this/path/should/not/exist");
  EXPECT_EQ(StatusCode::kNotFound, result.status().code());

  // Check that testing::TempDir() points to itself.
  static std::string kTempDir = CleanPath(testing::TempDir());
  result = RealPath(kTempDir);
  ASSERT_TRUE(result.ok());
  ASSERT_EQ(kTempDir, *result);

  // If so, check that RealPath resolves a known link.
  static std::string kLinkPath = testing::TempDir() + "link";
  ASSERT_EQ(std::error_code(), symlink(kTempDir.c_str(), kLinkPath.c_str()));
  result = RealPath(kLinkPath);
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(kTempDir, *result);
}

}  // namespace
}  // namespace kythe
