/*
 * Copyright 2020 The Kythe Authors. All rights reserved.
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
#include "kythe/cxx/common/regex.h"

#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "re2/re2.h"

namespace kythe {
namespace {

TEST(RegexTest, DefaultConstructorWorks) {
  Regex regex;
  EXPECT_TRUE(RE2::FullMatch("", regex));
}

TEST(RegexTest, MoveOperationsAreSane) {
  Regex orig = Regex::Compile("^hello_world$").value();
  EXPECT_TRUE(RE2::FullMatch("hello_world", orig));

  Regex dest = std::move(orig);
  EXPECT_TRUE(RE2::FullMatch("hello_world", dest));
  EXPECT_FALSE(RE2::FullMatch("hello_world", orig));
  EXPECT_TRUE(RE2::FullMatch("", orig));
}

TEST(RegexTest, CopyOperationsAreSane) {
  Regex orig = Regex::Compile("^hello_world$").value();
  EXPECT_TRUE(RE2::FullMatch("hello_world", orig));

  Regex dest = orig;
  EXPECT_TRUE(RE2::FullMatch("hello_world", dest));
  EXPECT_TRUE(RE2::FullMatch("hello_world", orig));
}

TEST(RegexTest, CopiesFromRE2) {
  RE2::Options options;
  // Use non-default options to ensure they are copied as well.
  options.set_literal(true);
  RE2 re("^hello_world$", options);

  Regex regex(re);

  EXPECT_FALSE(RE2::FullMatch("hello_world", re));
  EXPECT_FALSE(RE2::FullMatch("hello_world", regex));
  EXPECT_TRUE(RE2::FullMatch("^hello_world$", re));
  EXPECT_TRUE(RE2::FullMatch("^hello_world$", regex));
}

TEST(RegexSetTest, DefaultConstructorWorks) {
  RegexSet set;
  EXPECT_FALSE(set.Match(""));
}

TEST(RegexSetTest, MoveOperationsAreSane) {
  RegexSet orig = RegexSet::Build({"^hello_world$"}).value();
  EXPECT_TRUE(orig.Match("hello_world"));

  RegexSet dest = std::move(orig);
  EXPECT_TRUE(dest.Match("hello_world"));
  EXPECT_FALSE(orig.Match("hello_world"));
}

TEST(RegexSetTest, CopyOperationsAreSane) {
  RegexSet orig = RegexSet::Build({"^hello_world$"}).value();
  EXPECT_TRUE(orig.Match("hello_world"));

  RegexSet dest = orig;
  EXPECT_TRUE(dest.Match("hello_world"));
  EXPECT_TRUE(orig.Match("hello_world"));
}

TEST(RegexSetTest, ConstructsFromCompiledRE2Set) {
  RE2::Set base(RE2::DefaultOptions, RE2::UNANCHORED);
  ASSERT_EQ(base.Add("hello_world", nullptr), 0);
  ASSERT_TRUE(base.Compile());

  RegexSet set(std::move(base));
  EXPECT_TRUE(set.Match("hello_world"));
}

TEST(RegexSetTest, ExplainsMatch) {
  using ::testing::IsEmpty;
  using ::testing::UnorderedElementsAre;

  RE2::Options options;
  options.set_never_capture(true);
  RegexSet set = RegexSet::Build({"(hello_)?world", "(goodbye_)?world"},
                                 options, RE2::ANCHOR_BOTH)
                     .value();

  auto result = set.ExplainMatch("world");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, UnorderedElementsAre(0, 1));

  result = set.ExplainMatch("unmatched");
  ASSERT_TRUE(result.ok());
  EXPECT_THAT(*result, IsEmpty());
}

}  // namespace
}  // namespace kythe
