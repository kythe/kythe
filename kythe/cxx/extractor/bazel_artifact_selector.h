#ifndef KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_
#define KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_

#include <functional>
#include <memory>
#include <type_traits>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/meta/type_traits.h"
#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "google/protobuf/any.pb.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "re2/set.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

/// \brief BazelArtifactSelector is an interace which can be used for finding
/// extractor artifacts in a Bazel sequence of build_event_stream.BuildEvent
/// messages.
class BazelArtifactSelector {
 public:
  virtual ~BazelArtifactSelector() = default;

  /// \brief Selects matching BazelArtifacts from the provided event.
  /// Select() will be called for each message in the stream to allow
  /// implementations to update internal state.
  virtual absl::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) = 0;

  /// \brief Returns an Any-encoded protobuf with per-stream selector state.
  /// Stateful selectors should serialize any per-stream state into a
  /// suitable protocol buffer, encoded as an Any. If no state has been
  /// accumulated, they should return an empty protocol buffer of the
  /// appropriate type.
  /// Stateless selectors should return nullopt.
  virtual absl::optional<google::protobuf::Any> Serialize() const {
    return absl::nullopt;
  }

  /// \brief Finds and updates any per-stream state from the provided list.
  /// Stateless selectors should unconditionally return OK.
  /// Stateful selectors should return NotFound if the custom protobuf is not
  /// found in the provided list.
  virtual absl::Status Deserialize(
      absl::Span<const google::protobuf::Any> state) {
    return absl::OkStatus();
  }

 protected:
  // Not publicly copyable or movable to avoid slicing, but subclasses may be.
  BazelArtifactSelector() = default;
  BazelArtifactSelector(const BazelArtifactSelector&) = default;
  BazelArtifactSelector& operator=(const BazelArtifactSelector&) = default;
};

/// \brief A type-erased value-type implementation of the BazelArtifactSelector
/// interface. Does not derive from BazelArtifactSelector.
class AnyArtifactSelector {
 public:
  /// \brief Constructs an AnyArtifactSelector which delegates to the provided
  /// argument, which must derive from BazelArtifactSelector.
  template <
      typename S,
      typename = absl::enable_if_t<!std::is_same_v<S, AnyArtifactSelector>>,
      typename =
          absl::enable_if_t<std::is_convertible_v<S&, BazelArtifactSelector&>>>
  AnyArtifactSelector(S s)
      : AnyArtifactSelector([s = std::move(s)]() mutable -> S& { return s; }) {}

  // Copyable.
  AnyArtifactSelector(const AnyArtifactSelector&) = default;
  AnyArtifactSelector& operator=(const AnyArtifactSelector&) = default;

  /// \brief AnyArtifactSelector is movable, but will be empty after a move.
  /// The only valid operations on an empty AnyArtifactSelector is assigning a
  /// new value or destruction.
  AnyArtifactSelector(AnyArtifactSelector&&) = default;
  AnyArtifactSelector& operator=(AnyArtifactSelector&&) = default;

  /// \brief Forwards selection to the contained BazelArtifactSelector.
  absl::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) {
    return get_().Select(event);
  }

  /// \brief Forwards serialization to the contained BazelArtifactSelector.
  absl::optional<google::protobuf::Any> Serialize() const {
    return get_().Serialize();
  }

  /// \brief Forwards deserialization to the contained BazelArtifactSelector.
  absl::Status Deserialize(absl::Span<const google::protobuf::Any> state) {
    return get_().Deserialize(state);
  }

 private:
  explicit AnyArtifactSelector(std::function<BazelArtifactSelector&()> get)
      : get_(std::move(get)) {}

  std::function<BazelArtifactSelector&()> get_;
};

/// \brief Options class used for constructing an AspectArtifactSelector.
struct AspectArtifactSelectorOptions {
  // A set of patterns used to filter file names from NamedSetOfFiles events.
  RE2::Set file_name_allowlist;
  // A set of patterns used to filter output_group names from TargetComplete
  // events.
  RE2::Set output_group_allowlist;
  // A set of patterns used to filter aspect names from TargetComplete events.
  RE2::Set target_aspect_allowlist;
};

/// \brief A BazelArtifactSelector implementation which tracks state from
/// NamedSetOfFiles and TargetComplete events to select artifacts produced by
/// extractor aspects.
class AspectArtifactSelector final : public BazelArtifactSelector {
 public:
  using Options = AspectArtifactSelectorOptions;

  /// \brief Constructs an instance of AspectArtifactSelector from the provided
  /// options.
  explicit AspectArtifactSelector(Options options)
      : options_(std::make_shared<Options>(std::move(options))) {}

  /// \brief Selects an artifact if the event matches an expected
  /// aspect-produced compilation unit.
  absl::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) final;

  /// \brief Serializes the accumulated state into the return value, which will
  /// always be non-empty and of type
  /// `kythe.proto.BazelAspectArtifactSelectorState`.
  absl::optional<google::protobuf::Any> Serialize() const final;

  /// \brief Deserializes accumulated stream state from an Any of type
  /// `kythe.proto.BazelAspectArtifactSelectorState` or returns NotFound.
  absl::Status Deserialize(absl::Span<const google::protobuf::Any> state) final;

 private:
  struct State {
    // A record of all of the NamedSetOfFiles events which have been processed.
    absl::flat_hash_set<std::string> disposed;
    // Mapping from fileset id to NamedSetOfFiles whose file names matched the
    // allowlist, but have not yet been consumed by an event.
    absl::flat_hash_map<std::string, build_event_stream::NamedSetOfFiles>
        filesets;
    // Mapping from fileset id to target name which required that
    // file set when it had not yet been seen.
    absl::flat_hash_map<std::string, std::string> pending;
  };
  absl::optional<BazelArtifact> SelectFileSet(
      absl::string_view id, const build_event_stream::NamedSetOfFiles& fileset);

  absl::optional<BazelArtifact> SelectTargetCompleted(
      const build_event_stream::BuildEventId::TargetCompletedId& id,
      const build_event_stream::TargetComplete& payload);

  void ReadFilesInto(absl::string_view id, absl::string_view target,
                     std::vector<BazelArtifactFile>& files);

  // We must be copyable.
  std::shared_ptr<const Options> options_;
  State state_;
};

/// \brief An ArtifactSelector which selects artifacts emitted by extra
/// actions.
///
/// This will select any successful ActionCompleted build event, but the
/// selection can be restricted to an allowlist of action_types.
class ExtraActionSelector final : public BazelArtifactSelector {
 public:
  /// \brief Constructs an ExtraActionSelector from an allowlist against which
  /// to match ActionCompleted events. An empty set will select any successful
  /// action.
  explicit ExtraActionSelector(
      absl::flat_hash_set<std::string> action_types = {})
      : action_types_(std::move(action_types)) {}

  /// \brief Selects artifacts from ExtraAction-based extractors.
  absl::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) final;

 private:
  absl::flat_hash_set<std::string> action_types_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_
