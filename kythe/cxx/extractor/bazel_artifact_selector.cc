#include "kythe/cxx/extractor/bazel_artifact_selector.h"

#include "absl/strings/escaping.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "glog/logging.h"

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
          // data URIs use regualr base64, not "web safe" base64.
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

}  // namespace

absl::optional<BazelArtifact> AspectArtifactSelector::Select(
    const build_event_stream::BuildEvent& event) {
  if (event.id().has_named_set()) {
    return SelectFileSet(event.id().named_set().id(),
                         event.named_set_of_files());
  }
  if (event.id().has_target_completed()) {
    return SelectTargetCompleted(event.id().target_completed(),
                                 event.completed());
  }
  // TODO(shahms): Handle event.last_message == true
  return absl::nullopt;
}

absl::optional<google::protobuf::Any> AspectArtifactSelector::Serialize()
    const {
  return absl::nullopt;
}

absl::Status AspectArtifactSelector::Deserialize(
    absl::Span<const google::protobuf::Any> state) {
  return absl::OkStatus();
}

absl::optional<BazelArtifact> AspectArtifactSelector::SelectFileSet(
    absl::string_view id, const build_event_stream::NamedSetOfFiles& fileset) {
  bool kept = false;
  for (const auto& file : fileset.files()) {
    if (options_.file_name_allowlist.Match(file.name(), nullptr)) {
      kept = true;
      *filesets_contents_[id].add_files() = file;
    }
  }
  for (const auto& child : fileset.file_sets()) {
    if (!filesets_seen_.contains(child.id())) {
      kept = true;
      *filesets_contents_[id].add_file_sets() = child;
    }
  }
  // TODO(shahms): check pending *before* doing all of the insertion.
  if (auto iter = filesets_pending_.find(id); iter != filesets_pending_.end()) {
    auto node = filesets_pending_.extract(iter);
    BazelArtifact result = {.label = std::string(node.mapped())};
    ReadFilesInto(id, result.label, result.files);
    if (result.files.empty()) {
      return absl::nullopt;
    }
    return result;
  }
  if (!kept) {
    // There were no files, no children and no previous references, skip it.
    filesets_seen_.insert(std::string(id));
  }
  return absl::nullopt;
}

absl::optional<BazelArtifact> AspectArtifactSelector::SelectTargetCompleted(
    const build_event_stream::BuildEventId::TargetCompletedId& id,
    const build_event_stream::TargetComplete& payload) {
  if (payload.success() &&
      options_.target_aspect_allowlist.Match(id.aspect(), nullptr)) {
    BazelArtifact result = {
        .label = id.label(),
    };
    for (const auto& output_group : payload.output_group()) {
      if (options_.output_group_allowlist.Match(output_group.name(), nullptr)) {
        for (const auto& fileset : output_group.file_sets()) {
          ReadFilesInto(fileset.id(), id.label(), result.files);
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
  if (filesets_seen_.contains(id)) {
    return;
  }

  if (auto iter = filesets_contents_.find(id);
      iter != filesets_contents_.end()) {
    filesets_seen_.insert(std::string(id));
    auto node = filesets_contents_.extract(iter);
    const build_event_stream::NamedSetOfFiles& fileset = node.mapped();

    for (const auto& file : fileset.files()) {
      files.push_back({
          .local_path = AsLocalPath(file),
          .uri = AsUri(file),
      });
    }
    for (const auto& child : fileset.file_sets()) {
      ReadFilesInto(child.id(), target, files);
    }

    return;
  }

  // Files where requested, but we haven't seen that fileset id yet.  Record
  // this for future processing.
  LOG(INFO) << "NamedSetOfFiles " << id << " requested by " << target
            << " but not yet seen.";
  filesets_pending_.emplace(id, target);
}

}  // namespace kythe
