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
#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "glog/logging.h"
#include "google/protobuf/any.pb.h"
#include "kythe/proto/bazel_artifact_selector.pb.h"
#include "re2/re2.h"

namespace kythe {
namespace {

std::string AsUri(const build_event_stream::File& file) {
  switch (file.file_case()) {
    case build_event_stream::File::kUri:
      return file.uri();
    case build_event_stream::File::kContents:
      // We expect inline data to be rare and small, so always base64 encode it.
      return absl::StrCat(
          "data:base64,",
          // data URIs use regular base64, not "web safe" base64.
          absl::Base64Escape(file.contents()));
    case build_event_stream::File::FILE_NOT_SET:
      break;
  }
  LOG(FATAL) << "Unexpected build_event_stream::File case!" << file.file_case();
}

std::string AsLocalPath(const build_event_stream::File& file) {
  std::vector<std::string> parts(file.path_prefix().begin(),
                                 file.path_prefix().end());
  parts.push_back(file.name());
  return absl::StrJoin(parts, "/");
}

template <typename T>
struct FromRange {
  template <typename U>
  operator U() {
    return U(range.begin(), range.end());
  }

  const T& range;
};

template <typename T>
FromRange(const T&) -> FromRange<T>;

template <typename T>
const T& AsConstRef(const T& value) {
  return value;
}

template <typename T>
const T& AsConstRef(const T* value) {
  return *value;
}

template <typename T, typename U>
absl::Status DeserializeInternal(T& selector, const U& container) {
  absl::Status error;
  for (const auto& any : container) {
    switch (auto status = selector.DeserializeFrom(AsConstRef(any));
            status.code()) {
      case absl::StatusCode::kOk:
      case absl::StatusCode::kUnimplemented:
        return absl::OkStatus();
      case absl::StatusCode::kInvalidArgument:
        return status;
      case absl::StatusCode::kFailedPrecondition:
        error = status;
        continue;
      default:
        error = status;
        LOG(WARNING) << "Unrecognized status code: " << status;
    }
  }
  return error.ok() ? absl::NotFoundError("No state found")
                    : absl::NotFoundError(
                          absl::StrCat("No state found: ", error.ToString()));
}
}  // namespace

absl::Status BazelArtifactSelector::Deserialize(
    absl::Span<const google::protobuf::Any> state) {
  return DeserializeInternal(*this, state);
}

absl::Status BazelArtifactSelector::Deserialize(
    absl::Span<const google::protobuf::Any* const> state) {
  return DeserializeInternal(*this, state);
}

absl::optional<BazelArtifact> AspectArtifactSelector::Select(
    const build_event_stream::BuildEvent& event) {
  absl::optional<BazelArtifact> result = absl::nullopt;
  if (event.id().has_named_set()) {
    result =
        SelectFileSet(event.id().named_set().id(), event.named_set_of_files());
  } else if (event.id().has_target_completed()) {
    result =
        SelectTargetCompleted(event.id().target_completed(), event.completed());
  }
  if (event.last_message()) {
    state_ = {};
  }
  return result;
}

bool AspectArtifactSelector::SerializeInto(google::protobuf::Any& state) const {
  kythe::proto::BazelAspectArtifactSelectorState raw;
  *raw.mutable_disposed() = FromRange{state_.disposed};
  *raw.mutable_filesets() = FromRange{state_.filesets};
  *raw.mutable_pending() = FromRange{state_.pending};

  state.PackFrom(raw);
  return true;
}

absl::Status AspectArtifactSelector::DeserializeFrom(
    const google::protobuf::Any& state) {
  kythe::proto::BazelAspectArtifactSelectorState raw;
  if (state.UnpackTo(&raw)) {
    state_ = {
        .disposed = FromRange{raw.disposed()},
        .filesets = FromRange{raw.filesets()},
        .pending = FromRange{raw.pending()},
    };
    return absl::OkStatus();
  }
  if (state.Is<kythe::proto::BazelAspectArtifactSelectorState>()) {
    return absl::InvalidArgumentError(
        "Malformed kythe.proto.BazelAspectArtifactSelectorState");
  }
  return absl::FailedPreconditionError(
      "State not of type kythe.proto.BazelAspectArtifactSelectorState");
}

absl::optional<BazelArtifact> AspectArtifactSelector::SelectFileSet(
    absl::string_view id, const build_event_stream::NamedSetOfFiles& filesets) {
  bool kept = false;
  for (const auto& file : filesets.files()) {
    if (options_.file_name_allowlist.Match(file.name())) {
      kept = true;
      *state_.filesets[id].add_files() = file;
    }
  }
  for (const auto& child : filesets.file_sets()) {
    if (!state_.disposed.contains(child.id())) {
      kept = true;
      *state_.filesets[id].add_file_sets() = child;
    }
  }
  // TODO(shahms): check pending *before* doing all of the insertion.
  if (auto iter = state_.pending.find(id); iter != state_.pending.end()) {
    auto node = state_.pending.extract(iter);
    BazelArtifact result = {.label = std::string(node.mapped())};
    ReadFilesInto(id, result.label, result.files);
    if (result.files.empty()) {
      return absl::nullopt;
    }
    return result;
  }
  if (!kept) {
    // There were no files, no children and no previous references, skip it.
    state_.disposed.insert(std::string(id));
  }
  return absl::nullopt;
}

absl::optional<BazelArtifact> AspectArtifactSelector::SelectTargetCompleted(
    const build_event_stream::BuildEventId::TargetCompletedId& id,
    const build_event_stream::TargetComplete& payload) {
  if (payload.success() &&
      options_.target_aspect_allowlist.Match(id.aspect())) {
    BazelArtifact result = {
        .label = id.label(),
    };
    for (const auto& output_group : payload.output_group()) {
      if (options_.output_group_allowlist.Match(output_group.name())) {
        for (const auto& filesets : output_group.file_sets()) {
          ReadFilesInto(filesets.id(), id.label(), result.files);
        }
      }
    }
    if (!result.files.empty()) {
      return result;
    }
  }
  return absl::nullopt;
}

void AspectArtifactSelector::ReadFilesInto(
    absl::string_view id, absl::string_view target,
    std::vector<BazelArtifactFile>& files) {
  if (state_.disposed.contains(id)) {
    return;
  }

  if (auto iter = state_.filesets.find(id); iter != state_.filesets.end()) {
    state_.disposed.insert(std::string(id));
    auto node = state_.filesets.extract(iter);
    const build_event_stream::NamedSetOfFiles& filesets = node.mapped();

    for (const auto& file : filesets.files()) {
      files.push_back({
          .local_path = AsLocalPath(file),
          .uri = AsUri(file),
      });
    }
    for (const auto& child : filesets.file_sets()) {
      ReadFilesInto(child.id(), target, files);
    }

    return;
  }

  // Files where requested, but we haven't disposed that filesets id yet. Record
  // this for future processing.
  LOG(INFO) << "NamedSetOfFiles " << id << " requested by " << target
            << " but not yet disposed.";
  state_.pending.emplace(id, target);
}

ExtraActionSelector::ExtraActionSelector(
    absl::flat_hash_set<std::string> action_types)
    : action_matches_([action_types = std::move(action_types)](
                          absl::string_view action_type) {
        return action_types.empty() || action_types.contains(action_type);
      }) {}

ExtraActionSelector::ExtraActionSelector(const RE2* action_pattern)
    : action_matches_([action_pattern](absl::string_view action_type) {
        if (action_pattern == nullptr || action_pattern->pattern().empty()) {
          return false;
        }
        return RE2::FullMatch(action_type, *action_pattern);
      }) {
  CHECK(action_pattern == nullptr || action_pattern->ok())
      << "ExtraActionSelector requires a valid pattern: "
      << action_pattern->error();
}

absl::optional<BazelArtifact> ExtraActionSelector::Select(
    const build_event_stream::BuildEvent& event) {
  if (event.id().has_action_completed() && event.action().success() &&
      action_matches_(event.action().type())) {
    return BazelArtifact{
        .label = event.id().action_completed().label(),
        .files = {{
            .local_path = event.id().action_completed().primary_output(),
            .uri = AsUri(event.action().primary_output()),
        }},
    };
  }
  return absl::nullopt;
}

}  // namespace kythe
