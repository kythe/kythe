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

#include <string>

#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {
namespace {

std::string AsUri(const build_event_stream::File& primary_output) {
  switch (primary_output.file_case()) {
    case build_event_stream::File::kUri:
      return primary_output.uri();
    case build_event_stream::File::kContents:
      // We expect inline data to be rare and small, so always base64 encode it.
      return absl::StrCat(
          "data:base64,",
          // data URIs use regualr base64, not "web safe" base64.
          absl::Base64Escape(primary_output.contents()));
    case build_event_stream::File::FILE_NOT_SET:
      break;
  }
  LOG(FATAL) << "Unexpected build_event_stream::File case!"
             << primary_output.file_case();
}

std::vector<std::string> FileSetIds(
    const google::protobuf::RepeatedPtrField<
        build_event_stream::BuildEventId::NamedSetOfFilesId>& children) {
  std::vector<std::string> result;
  result.reserve(children.size());
  for (const auto& child : children) {
    result.push_back(child.id());
  }
  return result;
}
}  // namespace

void BazelArtifactReader::Next() {
  if (event_reader_->Done()) return;
  event_reader_->Next();
  Select();
}

void BazelArtifactReader::Select() {
  for (; !event_reader_->Done(); event_reader_->Next()) {
    const auto& event = event_reader_->Ref();
    for (auto& select : selectors_) {
      if (select(event, &absl::get<value_type>(value_))) {
        return;
      }
    }
  }
  // Both we and the underlying reader are done; propagate the status.
  value_ = event_reader_->status();
}

/* static */
std::vector<ArtifactSelector> BazelArtifactReader::DefaultSelectors() {
  return {ExtraActionSelector{}};
}

bool ExtraActionSelector::operator()(
    const build_event_stream::BuildEvent& event, BazelArtifact* result) const {
  if (event.id().has_action_completed() && event.action().success() &&
      (action_types.empty() || action_types.contains(event.action().type()))) {
    result->label = event.id().action_completed().label();
    result->uris = {AsUri(event.action().primary_output())};
    return true;
  }
  return false;
}

bool OutputGroupSelector::operator()(
    const build_event_stream::BuildEvent& event, BazelArtifact* result) {
  if (event.id().has_named_set()) {
    filesets_[event.id().named_set().id()] = event.named_set_of_files();
    return false;
  }
  if (!event.id().has_target_completed()) {
    return false;
  }
  std::vector<std::string> uris;
  for (const auto& group : event.completed().output_group()) {
    if (group_names_.contains(group.name())) {
      FindNamedUris(FileSetIds(group.file_sets()), &uris);
    }
  }
  if (!uris.empty()) {
    result->label = event.id().target_completed().label();
    result->uris = std::move(uris);
    return true;
  }
  return false;
}

void OutputGroupSelector::FindNamedUris(std::vector<std::string> ids,
                                        std::vector<std::string>* result) {
  while (!ids.empty()) {
    std::string id = std::move(ids.back());
    ids.pop_back();
    if (auto iter = filesets_.find(id); iter != filesets_.end()) {
      result->reserve(result->size() + iter->second.files().size());
      for (const auto& file : iter->second.files()) {
        result->push_back(AsUri(file));
      }

      ids.reserve(ids.size() + iter->second.file_sets().size());
      for (const auto& child : iter->second.file_sets()) {
        ids.push_back(child.id());
      }
    }
    // TODO(shahms): When do we want to remove visited file sets from the map?
    // The build event stream doesn't generally give us a reliable signal on
    // when it's safe to do so, but indefinitely tracking all seen filesets
    // is neither scalable nor resumable.
    // We can't drop them on first use, because that use might be from a file
    // group we don't care about prior to one we do and only doing so for
    // "selected" file groups solves neither problem.
    // We might be able to restrict it to groups with known suffixes (".kzip"),
    // and expire the groups on first visitation.
    // We also want to find a reliable way of determinition when no more outputs
    // will result from a particular target, but that doesn't seem possible with
    // extra actions.
  }
}

}  // namespace kythe
