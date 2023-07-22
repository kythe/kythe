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

#include <optional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/functional/any_invocable.h"
#include "absl/log/check.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "gmock/gmock.h"
#include "google/protobuf/any.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/text_format.h"
#include "gtest/gtest.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "kythe/proto/bazel_artifact_selector_v2.pb.h"
#include "protobuf-matchers/protocol-buffer-matchers.h"
#include "re2/re2.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {
namespace {
using ::protobuf_matchers::EqualsProto;
using ::testing::Eq;
using ::testing::FieldsAre;
using ::testing::IsEmpty;
using ::testing::Optional;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

struct FileSet {
  std::vector<std::string> files;
  std::vector<std::string> file_sets;
};

using IdGenerator = absl::AnyInvocable<std::string()>;

IdGenerator NumericIdGenerator() {
  return [i = 0]() mutable { return absl::StrCat(i++); };
}

IdGenerator AlphaIdGenerator() {
  return [i = 0]() mutable {
    std::string result = absl::StrCat(i++);
    for (char& ch : result) {
      ch = (ch - '0') + 'a';
    }
    return result;
  };
}

IdGenerator MixedIdGenerator() {
  return [numeric = NumericIdGenerator(), alpha = AlphaIdGenerator(),
          i = 0]() mutable { return (i++ % 2) ? numeric() : alpha(); };
}

absl::flat_hash_map<std::string, FileSet> GenerateFileSets(
    int count, IdGenerator& next_id) {
  absl::flat_hash_map<std::string, FileSet> result;
  for (int i = 0; i < count; ++i) {
    auto id = next_id();
    result[id].files = {absl::StrCat("path/to/file/", id, ".kzip")};
  }
  return result;
}

absl::flat_hash_map<std::string, FileSet> GenerateFileSets(
    int count, IdGenerator&& next_id) {
  return GenerateFileSets(count, next_id);
}

void ToNamedSetOfFilesEvents(
    const absl::flat_hash_map<std::string, FileSet>& file_sets,
    std::vector<build_event_stream::BuildEvent>& result) {
  result.reserve(result.size() + file_sets.size());
  for (const auto& [id, file_set] : file_sets) {
    auto& event = result.emplace_back();
    event.mutable_id()->mutable_named_set()->set_id(id);
    for (const auto& path : file_set.files) {
      auto* file = event.mutable_named_set_of_files()->add_files();
      file->set_name(path);
      file->set_uri(absl::StrCat("file:///", path));
    }
    for (const auto& child_id : file_set.file_sets) {
      event.mutable_named_set_of_files()->add_file_sets()->set_id(child_id);
    }
  }
}

std::vector<build_event_stream::BuildEvent> ToNamedSetOfFilesEvents(
    const absl::flat_hash_map<std::string, FileSet>& file_sets) {
  std::vector<build_event_stream::BuildEvent> result;
  ToNamedSetOfFilesEvents(file_sets, result);
  return result;
}

// The TargetCompleted event will always come after the NamedSetOfFiles events.
void ToTargetCompletedBuildEvents(
    absl::string_view label,
    const absl::flat_hash_map<std::string, FileSet>& file_sets,
    std::vector<build_event_stream::BuildEvent>& result) {
  ToNamedSetOfFilesEvents(file_sets, result);

  auto& event = result.emplace_back();
  event.mutable_id()->mutable_target_completed()->set_label(label);
  event.mutable_completed()->set_success(true);
  auto* output_group = event.mutable_completed()->add_output_group();
  for (const auto& [id, unused] : file_sets) {
    output_group->add_file_sets()->set_id(id);
  }
}

struct BuildEventOptions {
  int target_count = 10;
  int files_per_target = 2;
  int common_file_count = 2;
};

std::vector<build_event_stream::BuildEvent> GenerateBuildEvents(
    const BuildEventOptions& options = {},
    IdGenerator next_id = MixedIdGenerator()) {
  absl::flat_hash_map<std::string, FileSet> common =
      GenerateFileSets(options.common_file_count, next_id);

  std::vector<build_event_stream::BuildEvent> events;
  ToNamedSetOfFilesEvents(common, events);
  for (int i = 0; i < options.target_count; ++i) {
    absl::flat_hash_map<std::string, FileSet> files =
        GenerateFileSets(options.files_per_target, next_id);
    for (auto& [unused_id, file_set] : files) {
      for (const auto& [id, unused] : common) {
        file_set.file_sets.push_back(id);
      }
    }
    ToTargetCompletedBuildEvents(absl::StrCat("//path/to/target:", i), files,
                                 events);
  }
  return events;
}

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
              Eq(std::nullopt));
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
              Eq(std::nullopt));
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
              Eq(std::nullopt));
}

// Verify that we can find artifacts even if the target failed to build.
TEST(AspectArtifactSelectorTest, SelectsFailedTargets) {
  AspectArtifactSelector selector(DefaultOptions());

  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      success: false
      output_group {
        name: "kythe_compilation_unit"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
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

// Verify that we can find artifacts even if they were part of an unselected
// file group.
TEST(AspectArtifactSelectorTest, SelectsDuplicatedFileSets) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(std::nullopt));

  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
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
}

// Verify that we can find artifacts even if they were part of an unselected
// file group.
TEST(AspectArtifactSelectorTest, SelectsDuplicatedFileSetsWhenPruned) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  options.dispose_unselected_output_groups = true;
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(std::nullopt));

  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
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
}

TEST(AspectArtifactSelectorTest, SelectsSubsequentDuplicatedFileSets) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/some/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
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
}

TEST(AspectArtifactSelectorTest, SkipsSubsequentPrunedFileSets) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  options.dispose_unselected_output_groups = true;
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/some/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "kythe_compilation_unit"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
}

TEST(AspectArtifactSelectorTest, SkipsPrunedPendingFileSets) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  options.dispose_unselected_output_groups = true;
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/some/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(std::nullopt));
}

TEST(AspectArtifactSelectorTest, SkipsSubsequentPrunedPendingFileSets) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  options.dispose_unselected_output_groups = true;
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/some/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "kythe_compilation_unit"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(std::nullopt));
}

TEST(AspectArtifactSelectorTest, SelectsSubsequentPrunedPendingFileSets) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  options.dispose_unselected_output_groups = true;
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "kythe_compilation_unit"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/some/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
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

TEST(AspectArtifactSelectorTest, SkipsNonMatchingOutputGroups) {
  auto options = DefaultOptions();
  options.output_group_allowlist =
      RegexSet::Build({"kythe_compilation_unit"}).value();
  AspectArtifactSelector selector(options);
  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "1" } }
    named_set_of_files {
      files { name: "path/to/file.kzip" uri: "file:///path/to/file.kzip" }
    })pb")),
              Eq(std::nullopt));

  EXPECT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id {
      target_completed {
        label: "//path/to/target:name"
        aspect: "//aspect:file.bzl%name"
      }
    }
    completed {
      output_group {
        name: "unselected"
        file_sets { id: "1" }
      }
    })pb")),
              Eq(std::nullopt));
}

// Verifies that serializing integral ids in V2 format
// doesn't use the file_set_ids field.
TEST(AspectArtifactSelectorTest, SerializationOptimizesIntegers) {
  AspectArtifactSelector selector(DefaultOptions());

  std::vector<build_event_stream::BuildEvent> events =
      ToNamedSetOfFilesEvents(GenerateFileSets(5, NumericIdGenerator()));
  for (const auto& event : events) {
    ASSERT_EQ(selector.Select(event), std::nullopt);
  }
  google::protobuf::Any any;
  ASSERT_TRUE(selector.SerializeInto(any));
  kythe::proto::BazelAspectArtifactSelectorStateV2 state;
  ASSERT_TRUE(any.UnpackTo(&state));
  EXPECT_THAT(state.file_set_ids(), IsEmpty());
}

struct StressTestCase {
  enum class NamedSetIdStyle {
    kIntegral,
    kAlpha,
    kMixed,
  };
  NamedSetIdStyle id_style = NamedSetIdStyle::kMixed;
  bool reversed = false;
  AspectArtifactSelectorSerializationFormat serialization_format =
      AspectArtifactSelectorSerializationFormat::kV2;

  using TupleType = std::tuple<NamedSetIdStyle, bool,
                               AspectArtifactSelectorSerializationFormat>;
  explicit StressTestCase(const TupleType& t)
      : id_style(std::get<0>(t)),
        reversed(std::get<1>(t)),
        serialization_format(std::get<2>(t)) {}
};

class AspectArtifactSelectorStressTest
    : public testing::TestWithParam<StressTestCase> {
 public:
  std::vector<build_event_stream::BuildEvent> GenerateTestBuildEvents(
      const BuildEventOptions& options) {
    std::vector<build_event_stream::BuildEvent> events =
        GenerateBuildEvents(options, MakeIdGenerator());
    if (GetParam().reversed) {
      std::reverse(events.begin(), events.end());
    }
    return events;
  }

  IdGenerator MakeIdGenerator() {
    switch (GetParam().id_style) {
      case StressTestCase::NamedSetIdStyle::kIntegral:
        return NumericIdGenerator();
      case StressTestCase::NamedSetIdStyle::kAlpha:
        return AlphaIdGenerator();
      case StressTestCase::NamedSetIdStyle::kMixed:
        return MixedIdGenerator();
    }
  }
};

INSTANTIATE_TEST_SUITE_P(
    AspectArtifactSelectorStressTest, AspectArtifactSelectorStressTest,
    testing::ConvertGenerator<StressTestCase::TupleType>(testing::Combine(
        testing::Values(StressTestCase::NamedSetIdStyle::kIntegral,
                        StressTestCase::NamedSetIdStyle::kAlpha,
                        StressTestCase::NamedSetIdStyle::kMixed),
        testing::Bool(),
        testing::Values(AspectArtifactSelectorSerializationFormat::kV1,
                        AspectArtifactSelectorSerializationFormat::kV2))),
    [](const testing::TestParamInfo<
        AspectArtifactSelectorStressTest::ParamType>& info) {
      std::string id_style = [&] {
        switch (info.param.id_style) {
          case StressTestCase::NamedSetIdStyle::kAlpha:
            return "Alphabetic";
          case StressTestCase::NamedSetIdStyle::kIntegral:
            return "Integral";
          case StressTestCase::NamedSetIdStyle::kMixed:
            return "Mixed";
        }
      }();
      std::string format = [&] {
        switch (info.param.serialization_format) {
          case AspectArtifactSelectorSerializationFormat::kV1:
            return "V1";
          case AspectArtifactSelectorSerializationFormat::kV2:
            return "V2";
        }
      }();
      return absl::StrCat(info.param.reversed ? "Reversed" : "Ordered", "_",
                          id_style, "_", format);
    });

// Verify that the selector selects the expected number of files
// distributed across several targets.
TEST_P(AspectArtifactSelectorStressTest, SelectsExpectedTargetFiles) {
  std::vector<build_event_stream::BuildEvent> events = GenerateTestBuildEvents(
      {.target_count = 10, .files_per_target = 2, .common_file_count = 2});

  AspectArtifactSelector selector(DefaultOptions());
  absl::flat_hash_set<std::string> targets;
  absl::flat_hash_set<BazelArtifactFile> files;
  for (const auto& event : events) {
    if (auto artifact = selector.Select(event)) {
      targets.insert(artifact->label);
      files.insert(artifact->files.begin(), artifact->files.end());
    }
  }
  EXPECT_THAT(targets, SizeIs(10));
  EXPECT_THAT(files, SizeIs(10 * 2 + 2));
}

// Verify that the selector selects the expected number of files
// distributed across several targets, after deserialization into
// a freshly constructed selector.
TEST_P(AspectArtifactSelectorStressTest,
       SelectsExpectedTargetFilesWhenFreshlyDeserialized) {
  std::vector<build_event_stream::BuildEvent> events = GenerateTestBuildEvents(
      {.target_count = 10, .files_per_target = 2, .common_file_count = 2});

  absl::flat_hash_set<std::string> targets;
  absl::flat_hash_set<BazelArtifactFile> files;

  std::optional<google::protobuf::Any> state;
  for (const auto& event : events) {
    AspectArtifactSelector selector(DefaultOptions());
    if (state.has_value()) {
      ASSERT_EQ(selector.DeserializeFrom(*state), absl::OkStatus())
          << state->DebugString();
    }

    if (auto artifact = selector.Select(event)) {
      targets.insert(artifact->label);
      files.insert(artifact->files.begin(), artifact->files.end());
    }

    ASSERT_TRUE(selector.SerializeInto(state.emplace()));
  }
  EXPECT_THAT(targets, SizeIs(10));
  EXPECT_THAT(files, SizeIs(10 * 2 + 2));
}

// Verify that the selector selects the expected number of files
// distributed across several targets, after deserialization.
TEST_P(AspectArtifactSelectorStressTest,
       SelectsExpectedTargetFilesWhenDeserialized) {
  std::vector<build_event_stream::BuildEvent> events = GenerateTestBuildEvents(
      {.target_count = 10, .files_per_target = 2, .common_file_count = 2});

  absl::flat_hash_set<std::string> targets;
  absl::flat_hash_set<BazelArtifactFile> files;

  std::optional<google::protobuf::Any> state;
  AspectArtifactSelector selector(DefaultOptions());
  for (const auto& event : events) {
    if (state.has_value()) {
      ASSERT_EQ(selector.DeserializeFrom(*state), absl::OkStatus())
          << state->DebugString();
    }

    if (auto artifact = selector.Select(event)) {
      targets.insert(artifact->label);
      files.insert(artifact->files.begin(), artifact->files.end());
    }

    ASSERT_TRUE(selector.SerializeInto(state.emplace()));
  }
  EXPECT_THAT(targets, SizeIs(10));
  EXPECT_THAT(files, SizeIs(10 * 2 + 2));
}

TEST(AspectArtifactSelectorTest, SerializationRoundTrips) {
  AspectArtifactSelector selector(DefaultOptions());

  // Pending.
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
              Eq(std::nullopt));
  // Active.
  ASSERT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "2" } }
    named_set_of_files {
      files { name: "path/to/1/file.kzip" uri: "file:///path/to/1/file.kzip" }
      files { name: "path/to/2/file.kzip" uri: "file:///path/to/2/file.kzip" }
    })pb")),
              Eq(std::nullopt));

  // Active => Disposed.
  ASSERT_THAT(selector.Select(ParseEventOrDie(R"pb(
    id { named_set { id: "3" } }
    named_set_of_files {
      files { name: "path/to/1/file.kzip" uri: "file:///path/to/1/file.kzip" }
      files { name: "path/to/3/file.kzip" uri: "file:///path/to/3/file.kzip" }
    })pb")),
              Eq(std::nullopt));
  EXPECT_THAT(
      selector.Select(ParseEventOrDie(R"pb(
        id {
          target_completed {
            label: "//path/to/disposed/target:name"
            aspect: "//aspect:file.bzl%name"
          }
        }
        completed {
          success: true
          output_group {
            name: "kythe_compilation_unit"
            file_sets { id: "3" }
          }
        })pb")),
      Optional(FieldsAre(
          "//path/to/disposed/target:name",
          UnorderedElementsAre(
              FieldsAre("path/to/1/file.kzip", "file:///path/to/1/file.kzip"),
              FieldsAre("path/to/3/file.kzip",
                        "file:///path/to/3/file.kzip")))));

  google::protobuf::Any initial;
  ASSERT_TRUE(selector.SerializeInto(initial));
  {
    // The original selector round trips.
    ASSERT_THAT(selector.DeserializeFrom(initial), absl::OkStatus());

    google::protobuf::Any deserialized;
    ASSERT_TRUE(selector.SerializeInto(deserialized));

    EXPECT_THAT(deserialized, EqualsProto(initial));
  }

  {
    // A freshly constructed selector round trips.
    AspectArtifactSelector empty_selector(DefaultOptions());
    ASSERT_THAT(empty_selector.DeserializeFrom(initial), absl::OkStatus());

    google::protobuf::Any deserialized;
    ASSERT_TRUE(empty_selector.SerializeInto(deserialized));

    EXPECT_THAT(deserialized, EqualsProto(initial));
  }
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
                Eq(std::nullopt));
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
  MOCK_METHOD(std::optional<BazelArtifact>, Select,
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

  EXPECT_CALL(mock, Select(_)).WillOnce(Return(std::nullopt));
  EXPECT_CALL(mock, SerializeInto(Ref(dummy))).WillOnce(Return(false));
  EXPECT_CALL(mock, DeserializeFrom(Ref(dummy)))
      .WillOnce(Return(absl::UnimplementedError("")));

  AnyArtifactSelector selector = std::ref(mock);
  EXPECT_THAT(selector.Select(build_event_stream::BuildEvent()),
              Eq(std::nullopt));
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
              Eq(std::nullopt));
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
              Eq(std::nullopt));
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
              Eq(std::nullopt));
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
              Eq(std::nullopt));
}

}  // namespace
}  // namespace kythe
