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
#include "kythe/cxx/extractor/bazel_artifact_selector.h"

#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "glog/logging.h"
#include "gmock/gmock.h"
#include "google/protobuf/any.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "re2/re2.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {
namespace {
using ::testing::Eq;

AspectArtifactSelector::Options DefaultOptions() {
  return {
      .file_name_allowlist = RegexSet::Build({R"(\.kzip$)"}).value(),
      .output_group_allowlist = RegexSet::Build({".*"}).value(),
      .target_aspect_allowlist = RegexSet::Build({".*"}).value(),
  };
}

build_event_stream::BuildEvent ParseEventOrDie(absl::string_view text) {
  build_event_stream::BuildEvent result;

  google::protobuf::io::ArrayInputStream input(text.data(), text.size());
  CHECK(google::protobuf::TextFormat::Parse(&input, &result));
  return result;
}

// Verify that we can find artifacts when the fileset comes after the target.
TEST(AspectArtifactSelectorTest, SelectsOutOfOrderFileSets) {
  AspectArtifactSelector selector(DefaultOptions());

  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      success: true
      output_group {
        name: "kythe_compilation_unit"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(absl::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(BazelArtifact{
                  .label = "//path/to/target:name",
                  .files = {{
                      .local_path = "path/to/file.kzip",
                      .uri = "file:///path/to/file.kzip",
                  }},
              }));
}

TEST(AspectArtifactSelectorTest, SelectsMatchingTargetsOnce) {
  AspectArtifactSelector selector(DefaultOptions());

  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(absl::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      success: true
      output_group {
        name: "kythe_compilation_unit"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(BazelArtifact{
                  .label = "//path/to/target:name",
                  .files = {{
                      .local_path = "path/to/file.kzip",
                      .uri = "file:///path/to/file.kzip",
                  }},
              }));

  // Don't select the same fileset a second time, even for a different target.
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/new/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      success: true
      output_group {
        name: "kythe_compilation_unit"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(absl::nullopt));
}

TEST(AspectArtifactSelectorTest, CompatibleWithAny) {
  // Just ensures that AspectArtifactSelector can be assigned to an Any.
  AnyArtifactSelector unused = AspectArtifactSelector(DefaultOptions());
}

TEST(AspectArtifactSelectorTest, SerializationResumesSelection) {
  google::protobuf::Any state;
  {
    AspectArtifactSelector selector(DefaultOptions());

    EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
      id {
        target_completed {
          label: "//path/to/target:name"
          aspect: "//aspect:file.bzl%name"
        }
      }
      completed {
        success: true
        output_group {
          name: "kythe_compilation_unit"
          file_sets { id: "1" }
        }
      })pb")),
                Eq(absl::nullopt));
    ASSERT_THAT(selector.SerializeInto(state), true);
  }

  {
    AspectArtifactSelector selector(DefaultOptions());
    ASSERT_THAT(selector.Deserialize({state}), absl::OkStatus());

    EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
      id { named_set { id: "1" } }
      named_set_of_files {
        files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
      })pb")),
                Eq(BazelArtifact{
                    .label = "//path/to/target:name",
                    .files = {{
                        .local_path = "path/to/file.kzip",
                        .uri = "file:///path/to/file.kzip",
                    }},
                }));
  }
  {
    AspectArtifactSelector selector(DefaultOptions());
    ASSERT_THAT(selector.Deserialize({&state}), absl::OkStatus());

    EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
      id { named_set { id: "1" } }
      named_set_of_files {
        files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
      })pb")),
                Eq(BazelArtifact{
                    .label = "//path/to/target:name",
                    .files = {{
                        .local_path = "path/to/file.kzip",
                        .uri = "file:///path/to/file.kzip",
                    }},
                }));
  }
}

class MockArtifactSelector : public BazelArtifactSelector {
 public:
  MockArtifactSelector() = default;
  MOCK_METHOD(absl::optional<BazelArtifact>, Select,
              (const build_event_stream::BuildEvent&), (override));
  MOCK_METHOD(bool, SerializeInto, (google::protobuf::Any&), (const override));
  MOCK_METHOD(absl::Status, DeserializeFrom, (const google::protobuf::Any&),
              (override));
};

TEST(BazelArtifactSelectorTest, DeserializationProtocol) {
  using ::testing::Ref;
  using ::testing::Return;

  MockArtifactSelector mock;

  google::protobuf::Any any;

  EXPECT_CALL(mock, DeserializeFrom(Ref(any)))
      .WillOnce(Return(absl::OkStatus()))
      .WillOnce(Return(absl::UnimplementedError("")))
      .WillOnce(Return(absl::InvalidArgumentError("")))
      .WillOnce(Return(absl::FailedPreconditionError("")));

  EXPECT_THAT(mock.Deserialize({&any}), absl::OkStatus());
  EXPECT_THAT(mock.Deserialize({&any}), absl::OkStatus());
  EXPECT_THAT(mock.Deserialize({&any}), absl::InvalidArgumentError(""));
  EXPECT_THAT(mock.Deserialize({&any}),
              absl::NotFoundError("No state found: FAILED_PRECONDITION: "));

  EXPECT_CALL(mock, DeserializeFrom(Ref(any)))
      .WillOnce(Return(absl::FailedPreconditionError("")))
      .WillOnce(Return(absl::FailedPreconditionError("")))
      .WillOnce(Return(absl::OkStatus()));

  EXPECT_THAT(mock.Deserialize({&any, &any, &any}), absl::OkStatus());
}

TEST(AnyArtifactSelectorTest, ForwardsFunctions) {
  using ::testing::_;
  using ::testing::Ref;
  using ::testing::Return;

  MockArtifactSelector mock;
  google::protobuf::Any dummy;

  EXPECT_CALL(mock, Select(_)).WillOnce(Return(absl::nullopt));
  EXPECT_CALL(mock, SerializeInto(Ref(dummy))).WillOnce(Return(false));
  EXPECT_CALL(mock, DeserializeFrom(Ref(dummy)))
      .WillOnce(Return(absl::UnimplementedError("")));

  AnyArtifactSelector selector = std::ref(mock);
  EXPECT_THAT(selector.Select(build_event_stream::BuildEvent()),
              Eq(absl::nullopt));
  EXPECT_THAT(selector.SerializeInto(dummy), false);
  EXPECT_THAT(selector.DeserializeFrom(dummy), absl::UnimplementedError(""));
}

TEST(ExtraActionSelector, SelectsAllByDefault) {
  ExtraActionSelector selector;
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      action_completed {
        primary_output: "path/to/file/dummy.kzip"
        label: "//kythe/cxx/extractor:bazel_artifact_selector"
        configuration { id: "hash0" }
      }
    }
    action {
      success: true
      label: "//kythe/cxx/extractor:bazel_artifact_selector"
      primary_output { uri: "file:///home/path/to/file/dummy.kzip" }
      configuration { id: "hash0" }
      type: "extract_kzip_cxx_extra_action"
    }
  )pb")),
              Eq(BazelArtifact{
                  .label = "//kythe/cxx/extractor:bazel_artifact_selector",
                  .files = {{
                      .local_path = "path/to/file/dummy.kzip",
                      .uri = "file:///home/path/to/file/dummy.kzip",
                  }},
              }));
}

TEST(ExtraActionSelector, SelectsFromList) {
  ExtraActionSelector selector({"matching_action_type"});
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      action_completed {
        primary_output: "path/to/file/dummy.kzip"
        label: "//kythe/cxx/extractor:bazel_artifact_selector"
        configuration { id: "hash0" }
      }
    }
    action {
      success: true
      label: "//kythe/cxx/extractor:bazel_artifact_selector"
      primary_output { uri: "file:///home/path/to/file/dummy.kzip" }
      configuration { id: "hash0" }
      type: "matching_action_type"
    }
  )pb")),
              Eq(BazelArtifact{
                  .label = "//kythe/cxx/extractor:bazel_artifact_selector",
                  .files = {{
                      .local_path = "path/to/file/dummy.kzip",
                      .uri = "file:///home/path/to/file/dummy.kzip",
                  }},
              }));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      action_completed {
        primary_output: "path/to/file/dummy.kzip"
        label: "//kythe/cxx/extractor:bazel_artifact_selector"
        configuration { id: "hash0" }
      }
    }
    action {
      success: true
      label: "//kythe/cxx/extractor:bazel_artifact_selector"
      primary_output { uri: "file:///home/path/to/file/dummy.kzip" }
      configuration { id: "hash0" }
      type: "another_action_type"
    }
  )pb")),
              Eq(absl::nullopt));
}

TEST(ExtraActionSelector, SelectsFromPattern) {
  const RE2 pattern("matching_action_type");
  ExtraActionSelector selector(&pattern);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      action_completed {
        primary_output: "path/to/file/dummy.kzip"
        label: "//kythe/cxx/extractor:bazel_artifact_selector"
        configuration { id: "hash0" }
      }
    }
    action {
      success: true
      label: "//kythe/cxx/extractor:bazel_artifact_selector"
      primary_output { uri: "file:///home/path/to/file/dummy.kzip" }
      configuration { id: "hash0" }
      type: "matching_action_type"
    }
  )pb")),
              Eq(BazelArtifact{
                  .label = "//kythe/cxx/extractor:bazel_artifact_selector",
                  .files = {{
                      .local_path = "path/to/file/dummy.kzip",
                      .uri = "file:///home/path/to/file/dummy.kzip",
                  }},
              }));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      action_completed {
        primary_output: "path/to/file/dummy.kzip"
        label: "//kythe/cxx/extractor:bazel_artifact_selector"
        configuration { id: "hash0" }
      }
    }
    action {
      success: true
      label: "//kythe/cxx/extractor:bazel_artifact_selector"
      primary_output { uri: "file:///home/path/to/file/dummy.kzip" }
      configuration { id: "hash0" }
      type: "another_action_type"
    }
  )pb")),
              Eq(absl::nullopt));
}

TEST(ExtraActionSelector, SelectsNoneWithEmptyPattern) {
  const RE2 pattern("");
  ExtraActionSelector selector(&pattern);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      action_completed {
        primary_output: "path/to/file/dummy.kzip"
        label: "//kythe/cxx/extractor:bazel_artifact_selector"
        configuration { id: "hash0" }
      }
    }
    action {
      success: true
      label: "//kythe/cxx/extractor:bazel_artifact_selector"
      primary_output { uri: "file:///home/path/to/file/dummy.kzip" }
      configuration { id: "hash0" }
      type: "another_action_type"
    }
  )pb")),
              Eq(absl::nullopt));
}

TEST(ExtraActionSelector, SelectsNoneWithNullPattern) {
  ExtraActionSelector selector(nullptr);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      action_completed {
        primary_output: "path/to/file/dummy.kzip"
        label: "//kythe/cxx/extractor:bazel_artifact_selector"
        configuration { id: "hash0" }
      }
    }
    action {
      success: true
      label: "//kythe/cxx/extractor:bazel_artifact_selector"
      primary_output { uri: "file:///home/path/to/file/dummy.kzip" }
      configuration { id: "hash0" }
      type: "another_action_type"
    }
  )pb")),
              Eq(absl::nullopt));
}

}  // namespace
}  // namespace kythe
