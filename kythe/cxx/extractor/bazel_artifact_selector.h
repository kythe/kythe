#ifndef KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_
#define KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "google/protobuf/any.pb.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "re2/set.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

class BazelArtifactSelector {
 public:
  virtual ~BazelArtifactSelector() = default;

  virtual absl::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) = 0;

  virtual absl::optional<google::protobuf::Any> Serialize() const {
    return absl::nullopt;
  }

  virtual absl::Status Deserialize(
      absl::Span<const google::protobuf::Any> state) {
    return absl::OkStatus();
  }

 protected:
  BazelArtifactSelector() = default;
  BazelArtifactSelector(const BazelArtifactSelector&) = default;
  BazelArtifactSelector& operator=(const BazelArtifactSelector&) = default;
};

struct AspectArtifactSelectorOptions {
  RE2::Set file_name_allowlist;
  RE2::Set output_group_allowlist;
  RE2::Set target_aspect_allowlist;
};

class AspectArtifactSelector final : public BazelArtifactSelector {
 public:
  using Options = AspectArtifactSelectorOptions;

  explicit AspectArtifactSelector(Options options)
      : options_(std::move(options)) {}

  absl::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) override;

  absl::optional<google::protobuf::Any> Serialize() const override;

  absl::Status Deserialize(
      absl::Span<const google::protobuf::Any> state) override;

 private:
  absl::optional<BazelArtifact> SelectFileSet(
      absl::string_view id, const build_event_stream::NamedSetOfFiles& fileset);

  absl::optional<BazelArtifact> SelectTargetCompleted(
      const build_event_stream::BuildEventId::TargetCompletedId& id,
      const build_event_stream::TargetComplete& payload);

  void ReadFilesInto(absl::string_view id, absl::string_view target,
                     std::vector<BazelArtifactFile>& files);

  Options options_;

  // A record of all of the NamedSetOfFiles events which have been seen so far.
  absl::flat_hash_set<std::string> filesets_seen_;
  // Mapping from fileset id to NamedSetOfFiles whose file names matched the
  // allowlist, but have not yet been consumed by an event.
  absl::flat_hash_map<std::string, build_event_stream::NamedSetOfFiles>
      filesets_contents_;
  // Mapping from fileset id to target name which required that
  // file set when it had not yet been seen.
  absl::flat_hash_map<std::string, std::string> filesets_pending_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_
