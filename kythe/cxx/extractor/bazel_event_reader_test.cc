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
#include "kythe/cxx/extractor/bazel_event_reader.h"

#include <sstream>

#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "gtest/gtest.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {
namespace {
using ::google::protobuf::io::IstreamInputStream;
using ::google::protobuf::util::SerializeDelimitedToOstream;
using ::testing::ElementsAre;

TEST(BazelEventReaderTest, ReadsExpectedEvents) {
  std::stringstream stream;
  for (int i = 0; i < 5; i++) {
    build_event_stream::BuildEvent event;
    event.mutable_id()->mutable_unknown()->set_details(absl::StrCat(i));
    ASSERT_TRUE(SerializeDelimitedToOstream(event, &stream));
  }
  IstreamInputStream input_stream(&stream);

  std::vector<std::string> events;
  BazelEventReader reader(&input_stream);
  for (; !reader.Done(); reader.Next()) {
    events.push_back(reader.Ref().id().unknown().details());
  }
  ASSERT_TRUE(reader.status().ok()) << reader.status();
  EXPECT_THAT(events, ElementsAre("0", "1", "2", "3", "4"));
}

}  // namespace
}  // namespace kythe
