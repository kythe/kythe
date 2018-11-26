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

#include <string>

#include "gtest/gtest.h"
#include "kythe/cxx/common/path_utils.h"

namespace kythe {
namespace {

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

}  // namespace
}  // namespace kythe
