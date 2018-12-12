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

#include "kythe/cxx/indexer/proto/comments.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {

using ::testing::Eq;

TEST(CommentsTest, Empty) { EXPECT_THAT(StripCommentMarkers(""), Eq("")); }

TEST(CommentsTest, LeadingMarker1) {
  EXPECT_THAT(StripCommentMarkers("// foo"), Eq("foo"));
  EXPECT_THAT(StripCommentMarkers("  // foo"), Eq("foo"));
}

TEST(CommentsTest, TrailingSpace) {
  EXPECT_THAT(StripCommentMarkers(" "), Eq(""));
  EXPECT_THAT(StripCommentMarkers("bar "), Eq("bar"));
}

TEST(CommentsTest, LeadingAndTrailing1) {
  EXPECT_THAT(StripCommentMarkers("// foo  "), Eq("foo"));
  EXPECT_THAT(StripCommentMarkers("  // foo  "), Eq("foo"));
}

TEST(CommentsTest, LeadingMarkers) {
  EXPECT_THAT(StripCommentMarkers("// a b c\n"
                                  "  // d e f\n"
                                  "  // g"),
              Eq("a b c\n"
                 "d e f\n"
                 "g"));
}

TEST(CommentsTest, LeadingAndTrailing) {
  EXPECT_THAT(StripCommentMarkers(" // the days are bright \n"
                                  " // and filled with pain \t\n"
                                  "// enclose me in your gentle  \n"
                                  " //\n"
                                  "  rain "),
              Eq("the days are bright\n"
                 "and filled with pain\n"
                 "enclose me in your gentle\n"
                 "\n"
                 "rain"));
}

TEST(CommentsTest, BlockComment) {
  EXPECT_THAT(StripCommentMarkers("/* all your base\n"
                                  " * are belong\n"
                                  " *\n"
                                  " * to us\n"
                                  " */"),
              Eq("all your base\n"
                 "are belong\n"
                 "\n"
                 "to us\n"));
}

}  // namespace
}  // namespace kythe
