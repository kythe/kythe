/*
 * Copyright 2023 The Kythe Authors. All rights reserved.
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
#include "kythe/cxx/indexer/textproto/recordio_textparser.h"

#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace kythe {
namespace {
using ::testing::ElementsAre;
using ::testing::Pair;

std::vector<std::pair<int, std::string>> ParsedChunks(std::string content,
                                                      std::string separator) {
  std::vector<std::pair<int, std::string>> chunks;
  lang_textproto::ParseRecordTextChunks(
      content, separator, [&](absl::string_view chunk, int chunk_start_line) {
        chunks.emplace_back(chunk_start_line, std::string(chunk));
      });
  return chunks;
}

TEST(RecordioTextparserTest, TwoRecordSeparated) {
  std::string content = absl::StrJoin({"item: 1", "---", "item: 2"}, "\n");
  std::vector<std::pair<int, std::string>> chunks =
      ParsedChunks(content, "---");

  EXPECT_THAT(chunks, ElementsAre(Pair(0, "item: 1\n"), Pair(2, "item: 2")));
}

TEST(RecordioTextparserTest, CommentsAlsoSeparatedByNewline) {
  std::string content = absl::StrJoin(
      {"# Comment1", "", "# Comment2", "item: 1", "", "# Comment3", "item: 2"},
      "\n");
  std::vector<std::pair<int, std::string>> chunks = ParsedChunks(content, "\n");

  EXPECT_THAT(chunks,
              ElementsAre(Pair(0, "# Comment1\n\n# Comment2\nitem: 1\n"),
                          Pair(5, "# Comment3\nitem: 2")));
}

TEST(RecordioTextparserTest, SeparatorStartsWithComment) {
  std::string content =
      absl::StrJoin({"# Comment1", " # --- ", "# Comment2", "item: 1", "# ---",
                     "# Comment3", "item: 2"},
                    "\n");
  std::vector<std::pair<int, std::string>> chunks =
      ParsedChunks(content, "# ---");

  EXPECT_THAT(chunks,
              ElementsAre(Pair(0, "# Comment1\n # --- \n# Comment2\nitem: 1\n"),
                          Pair(5, "# Comment3\nitem: 2")));
}

TEST(RecordioTextparserTest, EndsWithComment) {
  std::string content = absl::StrJoin(
      {"# Comment1", "item: 1", "", "item: 2", "# Comment2"}, "\n");
  std::vector<std::pair<int, std::string>> chunks = ParsedChunks(content, "\n");

  EXPECT_THAT(chunks, ElementsAre(Pair(0, "# Comment1\nitem: 1\n"),
                                  Pair(3, "item: 2\n# Comment2")));
}

}  // namespace
}  // namespace kythe
