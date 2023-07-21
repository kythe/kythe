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
#include "kythe/cxx/extractor/bazel_artifact_reader.h"

#include <utility>

#include "absl/strings/str_cat.h"
#include "gmock/gmock.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/bazel_event_reader.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {
namespace {
using ::google::protobuf::io::IstreamInputStream;
using ::google::protobuf::util::SerializeDelimitedToOstream;
using ::testing::ElementsAre;

template <typename T, typename U>
auto Artifact(T&& t, U&& u) {
  return ::testing::AllOf(
      ::testing::Field(&BazelArtifact::label, std::forward<T>(t)),
      ::testing::Field(&BazelArtifact::files, std::forward<U>(u)));
}

template <typename T, typename U>
auto File(T&& path, U&& uri) {
  return ::testing::AllOf(
      ::testing::Field(&BazelArtifactFile::local_path, std::forward<T>(path)),
      ::testing::Field(&BazelArtifactFile::uri, std::forward<U>(uri)));
}

TEST(BazelArtifactReaderTest, ReadsExpectedArtifacts) {
  static constexpr int kArtifactCount = 5;
  std::stringstream stream;
  for (int i = 0; i < kArtifactCount; i++) {
    build_event_stream::BuildEvent event;
    auto* id = event.mutable_id()->mutable_action_completed();
    id->set_primary_output("local/path/to/file.kzip");
    id->set_label(absl::StrCat("//id/label:", i));
    auto* payload = event.mutable_action();
    payload->set_success(true);
    payload->set_type("extra_action");
    payload->mutable_primary_output()->set_uri(
        absl::StrCat("file:///dummy/", i, ".kzip"));
    ASSERT_TRUE(SerializeDelimitedToOstream(event, &stream));
  }

  IstreamInputStream input(&stream);
  BazelEventReader events(&input);
  BazelArtifactReader reader(&events);
  std::vector<BazelArtifact> artifacts;
  for (; !reader.Done(); reader.Next()) {
    artifacts.push_back(std::move(reader.Ref()));
  }
  ASSERT_TRUE(reader.status().ok()) << reader.status();
  EXPECT_THAT(
      artifacts,
      ElementsAre(
          Artifact("//id/label:0", ElementsAre(File("local/path/to/file.kzip",
                                                    "file:///dummy/0.kzip"))),
          Artifact("//id/label:1", ElementsAre(File("local/path/to/file.kzip",
                                                    "file:///dummy/1.kzip"))),
          Artifact("//id/label:2", ElementsAre(File("local/path/to/file.kzip",
                                                    "file:///dummy/2.kzip"))),
          Artifact("//id/label:3", ElementsAre(File("local/path/to/file.kzip",
                                                    "file:///dummy/3.kzip"))),
          Artifact("//id/label:4", ElementsAre(File("local/path/to/file.kzip",
                                                    "file:///dummy/4.kzip")))));
}

}  // namespace
}  // namespace kythe
