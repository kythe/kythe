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

#include "json_proto.h"

#include <string>

#include "absl/log/initialize.h"
#include "google/protobuf/stubs/common.h"
#include "gtest/gtest.h"
#include "kythe/proto/analysis.pb.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {
namespace {
TEST(JsonProto, Serialize) {
  proto::FileData file_data;
  std::string data_out;
  ASSERT_TRUE(WriteMessageAsJsonToString(file_data, "kythe", &data_out));
  EXPECT_EQ("{\"format\":\"kythe\",\"content\":{}}", data_out);
  file_data.set_content("text");
  file_data.mutable_info()->set_path("here");
  data_out.clear();
  ASSERT_TRUE(WriteMessageAsJsonToString(file_data, "kythe", &data_out));
  EXPECT_EQ(
      "{\"format\":\"kythe\",\"content\":{\"content\":\"dGV4dA==\",\"info\":{"
      "\"path\":\"here\"}}}",
      data_out);
  proto::Entries has_repeated_field;
  data_out.clear();
  ASSERT_TRUE(
      WriteMessageAsJsonToString(has_repeated_field, "kythe", &data_out));
  EXPECT_EQ("{\"format\":\"kythe\",\"content\":{}}", data_out);
  has_repeated_field.add_entries()->set_edge_kind("e1");
  has_repeated_field.add_entries();
  data_out.clear();
  ASSERT_TRUE(
      WriteMessageAsJsonToString(has_repeated_field, "kythe", &data_out));
  EXPECT_EQ(
      "{\"format\":\"kythe\",\"content\":{\"entries\":[{\"edge_kind\":\"e1\"},{"
      "}]}}",
      data_out);
}

TEST(JsonProto, Deserialize) {
  proto::FileData file_data;
  std::string format_string;
  ASSERT_FALSE(MergeJsonWithMessage("{}", &format_string, &file_data));
  ASSERT_FALSE(MergeJsonWithMessage("{\"format\":{},\"content\":{}}",
                                    &format_string, &file_data));
  ASSERT_FALSE(MergeJsonWithMessage("{\"format\":\"wrong\",\"content\":{}}",
                                    &format_string, &file_data));
  ASSERT_FALSE(
      MergeJsonWithMessage("{\"content\":{}}", &format_string, &file_data));
  ASSERT_TRUE(MergeJsonWithMessage("{\"format\":\"kythe\",\"content\":{}}",
                                   &format_string, &file_data));
  EXPECT_EQ("kythe", format_string);
  ASSERT_TRUE(MergeJsonWithMessage(
      "{\"format\":\"kythe\",\"content\":{\"content\":\"dGV4dA==\",\"info\":{"
      "\"path\":\"here\"}}}",
      &format_string, &file_data));
  EXPECT_EQ("text", file_data.content());
  EXPECT_EQ("here", file_data.info().path());
  proto::Entries has_repeated_field;
  ASSERT_TRUE(MergeJsonWithMessage(
      "{\"format\":\"kythe\",\"content\":{\"entries\":[]}}", &format_string,
      &has_repeated_field));
  EXPECT_EQ(0, has_repeated_field.entries_size());
  ASSERT_TRUE(
      MergeJsonWithMessage("{\"format\":\"kythe\",\"content\":{\"entries\":[{"
                           "\"edge_kind\":\"e0\"},{\"edge_kind\":\"e2\"}]}}",
                           &format_string, &has_repeated_field));
  EXPECT_EQ(2, has_repeated_field.entries_size());
  EXPECT_EQ("e0", has_repeated_field.entries(0).edge_kind());
  EXPECT_EQ("e2", has_repeated_field.entries(1).edge_kind());
}

}  // namespace
}  // namespace kythe

int main(int argc, char** argv) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  absl::InitializeLog();
  ::testing::InitGoogleTest(&argc, argv);
  int result = RUN_ALL_TESTS();
  return result;
}
