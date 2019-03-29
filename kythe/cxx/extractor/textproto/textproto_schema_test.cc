/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/extractor/textproto/textproto_schema.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace kythe {
namespace lang_textproto {
namespace {

using ::testing::ElementsAreArray;
using ::testing::Eq;

TEST(TextprotoSchemaTest, Empty) {
  auto schema = ParseTextprotoSchemaComments("");
  EXPECT_THAT(schema.proto_file, Eq(""));
  EXPECT_THAT(schema.proto_message, Eq(""));
}

TEST(TextprotoSchemaTest, ProtoFile) {
  auto schema = ParseTextprotoSchemaComments("# proto-file: some/file.proto");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq(""));
}

TEST(TextprotoSchemaTest, ProtoFileNoSpaces) {
  auto schema = ParseTextprotoSchemaComments("#proto-file:some/file.proto");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq(""));
}

TEST(TextprotoSchemaTest, ProtoFileExtraSpaces) {
  auto schema =
      ParseTextprotoSchemaComments("#  \t  proto-file: \tsome/file.proto");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq(""));
}

TEST(TextprotoSchemaTest, ProtoMessage) {
  auto schema =
      ParseTextprotoSchemaComments("# proto-message: my_namespace.MyMessage");
  EXPECT_THAT(schema.proto_file, Eq(""));
  EXPECT_THAT(schema.proto_message, Eq("my_namespace.MyMessage"));
}

TEST(TextprotoSchemaTest, ProtoMessageAndFile) {
  auto schema = ParseTextprotoSchemaComments(
      "# proto-message: my_namespace.MyMessage\n"
      "# proto-file: some/file.proto");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq("my_namespace.MyMessage"));
}

TEST(TextprotoSchemaTest, LeadingNewline) {
  auto schema = ParseTextprotoSchemaComments(
      "\n# proto-message: my_namespace.MyMessage\n"
      "# proto-file: some/file.proto");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq("my_namespace.MyMessage"));
}

TEST(TextprotoSchemaTest, ExtraNewlines) {
  auto schema = ParseTextprotoSchemaComments(
      "\n# proto-message: my_namespace.MyMessage\n\n\n"
      "# proto-file: some/file.proto");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq("my_namespace.MyMessage"));
}

TEST(TextprotoSchemaTest, ProtoImports) {
  auto schema = ParseTextprotoSchemaComments(
      "# proto-file: some/file.proto\n\n"
      "#proto-import: my_extensions.proto\n"
      "# proto-import:  other/extensions.proto");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq(""));
  EXPECT_THAT(
      schema.proto_imports,
      ElementsAreArray({"my_extensions.proto", "other/extensions.proto"}));
}

TEST(TextprotoSchemaTest, StopSearchingAfterContent) {
  auto schema = ParseTextprotoSchemaComments(
      "# proto-file: some/file.proto\n"
      "field: \"value\"\n"
      "# proto-message: MyMessage\n");
  EXPECT_THAT(schema.proto_file, Eq("some/file.proto"));
  EXPECT_THAT(schema.proto_message, Eq(""));
}

}  // namespace
}  // namespace lang_textproto
}  // namespace kythe
