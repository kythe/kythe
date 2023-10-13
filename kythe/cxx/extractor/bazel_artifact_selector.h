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
#ifndef KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_
#define KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/inlined_vector.h"
#include "absl/container/node_hash_map.h"
#include "absl/meta/type_traits.h"
#include "absl/status/status.h"
#include "absl/types/span.h"
#include "google/protobuf/any.pb.h"
#include "kythe/cxx/common/regex.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "re2/re2.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

/// \brief BazelArtifactSelector is an interface which can be used for finding
/// extractor artifacts in a Bazel sequence of build_event_stream.BuildEvent
/// messages.
class BazelArtifactSelector {
 public:
  virtual ~BazelArtifactSelector() = default;

  /// \brief Selects matching BazelArtifacts from the provided event.
  /// Select() will be called for each message in the stream to allow
  /// implementations to update internal state.
  virtual std::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) = 0;

  /// \brief Encodes per-stream selector state into the Any protobuf.
  /// Stateful selectors should serialize any per-stream state into a
  /// suitable protocol buffer, encoded as an Any. If no state has been
  /// accumulated, they should return an empty protocol buffer of the
  /// appropriate type and return true.
  /// Stateless selectors should return false.
  virtual bool SerializeInto(google::protobuf::Any& state) const {
    return false;
  }

  /// \brief Updates any per-stream state from the provided proto.
  /// Stateless selectors should unconditionally return a kUnimplemented status.
  /// Stateful selectors should return OK if the provided state contains a
  /// suitable proto, InvalidArgument if the proto is of the right type but
  /// cannot be decoded or FailedPrecondition if the proto is of the wrong type.
  virtual absl::Status DeserializeFrom(const google::protobuf::Any& state) {
    return absl::UnimplementedError("stateless selector");
  }

  /// \brief Finds and updates any per-stream state from the provided list.
  /// Returns OK if the selector is stateless or if the requisite state was
  /// found in the list.
  /// Returns NotFound for a stateful selector whose state was not present
  /// or InvalidArgument if the state was present but couldn't be decoded.
  absl::Status Deserialize(absl::Span<const google::protobuf::Any> state);
  absl::Status Deserialize(
      absl::Span<const google::protobuf::Any* const> state);

 protected:
  // Not publicly copyable or movable to avoid slicing, but subclasses may be.
  BazelArtifactSelector() = default;
  BazelArtifactSelector(const BazelArtifactSelector&) = default;
  BazelArtifactSelector& operator=(const BazelArtifactSelector&) = default;
};

/// \brief A type-erased value-type implementation of the BazelArtifactSelector
/// interface.
class AnyArtifactSelector final : public BazelArtifactSelector {
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
  std::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) {
    return get_().Select(event);
  }

  /// \brief Forwards serialization to the contained BazelArtifactSelector.
  bool SerializeInto(google::protobuf::Any& state) const final {
    return get_().SerializeInto(state);
  }

  /// \brief Forwards deserialization to the contained BazelArtifactSelector.
  absl::Status DeserializeFrom(const google::protobuf::Any& state) final {
    return get_().DeserializeFrom(state);
  }

 private:
  explicit AnyArtifactSelector(std::function<BazelArtifactSelector&()> get)
      : get_(std::move(get)) {}

  std::function<BazelArtifactSelector&()> get_;
};

/// \brief Known serialization format versions.
enum class AspectArtifactSelectorSerializationFormat {
  kV1,  // The initial, bulky-but-simple format.
  kV2,  // The newer, flatter, smaller format.
};

/// \brief Options class used for constructing an AspectArtifactSelector.
struct AspectArtifactSelectorOptions {
  // A set of patterns used to filter file names from NamedSetOfFiles events.
  // Matches nothing by default.
  RegexSet file_name_allowlist;
  // A set of patterns used to filter output_group names from TargetComplete
  // events. Matches nothing by default.
  RegexSet output_group_allowlist;
  // A set of patterns used to filter aspect names from TargetComplete events.
  RegexSet target_aspect_allowlist = RegexSet::Build({".*"}).value();
  // Which serialization format version to use.
  AspectArtifactSelectorSerializationFormat serialization_format =
      AspectArtifactSelectorSerializationFormat::kV2;
  // Whether to eagerly drop files and filesets from unselected output groups.
  // As this can cause data loss when a file set would have been selected
  // by a subsequent target's output group, it defaults to false.
  bool dispose_unselected_output_groups = false;
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
      : options_(std::move(options)) {}

  AspectArtifactSelector(const AspectArtifactSelector&) = default;
  AspectArtifactSelector& operator=(const AspectArtifactSelector&) = default;
  AspectArtifactSelector(AspectArtifactSelector&&) = default;
  AspectArtifactSelector& operator=(AspectArtifactSelector&&) = default;

  /// \brief Selects an artifact if the event matches an expected
  /// aspect-produced compilation unit.
  std::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) final;

  /// \brief Serializes the accumulated state into the return value, which will
  /// always be non-empty and of type
  /// `kythe.proto.BazelAspectArtifactSelectorState`.
  bool SerializeInto(google::protobuf::Any& state) const final;

  /// \brief Deserializes accumulated stream state from an Any of type
  /// `kythe.proto.BazelAspectArtifactSelectorState`.
  absl::Status DeserializeFrom(const google::protobuf::Any& state) final;

 private:
  friend class AspectArtifactSelectorSerializationHelper;

  using FileId = std::tuple<uint64_t>;
  using FileSetId = std::tuple<int64_t>;

  class FileTable {
   public:
    FileTable() = default;
    FileTable(const FileTable& other);
    FileTable& operator=(const FileTable& other);
    FileTable(FileTable&&) = default;
    FileTable& operator=(FileTable&&) = default;

    FileId Insert(BazelArtifactFile file);
    std::optional<BazelArtifactFile> Extract(FileId id);
    // Extract the equivalent file, if present, returning the argument.
    BazelArtifactFile ExtractFile(BazelArtifactFile file);

    const BazelArtifactFile* Find(FileId) const;

    auto begin() const { return id_map_.begin(); }
    auto end() const { return id_map_.end(); }

   private:
    struct Entry {
      FileId id;
      int count = 0;
    };
    using FileMap = absl::node_hash_map<BazelArtifactFile, Entry>;
    using IdMap = absl::flat_hash_map<FileId, const BazelArtifactFile*>;

    BazelArtifactFile ExtractIterators(IdMap::iterator id_iter,
                                       FileMap::iterator file_iter);

    uint64_t next_id_ = 0;
    FileMap file_map_;
    IdMap id_map_;
  };

  struct FileSet {
    absl::InlinedVector<FileId, 1> files;
    absl::InlinedVector<FileSetId, 1> file_sets;
  };

  class FileSetTable {
   public:
    std::optional<FileSetId> InternUnlessDisposed(absl::string_view id);
    bool InsertUnlessDisposed(FileSetId id, FileSet file_set);
    // Extracts the FileSet and, if previously present, marks it disposed.
    std::optional<FileSet> ExtractAndDispose(FileSetId id);
    // Unconditionally marks a FileSet as disposed.
    // Erases it if present in the map.
    void Dispose(FileSetId id);
    [[nodiscard]] bool Disposed(FileSetId id);

    std::string ToString(FileSetId id) const;

    const absl::flat_hash_map<FileSetId, FileSet>& file_sets() const {
      return file_sets_;
    }
    const absl::flat_hash_set<FileSetId>& disposed() const { return disposed_; }

   private:
    std::pair<FileSetId, bool> InternOrCreate(absl::string_view id);

    // A record of all pending FileSets.
    absl::flat_hash_map<FileSetId, FileSet> file_sets_;
    // A record of all of the NamedSetOfFiles events which have been processed.
    absl::flat_hash_set<FileSetId> disposed_;

    // The next integral id to use.
    // Non-integral file set ids are mapped to negative values.
    int64_t next_id_ = -1;
    // For non-integral file set ids coming from Bazel.
    absl::flat_hash_map<std::string, FileSetId> id_map_;
    absl::flat_hash_map<FileSetId, std::string> inverse_id_map_;
  };

  struct State {
    // A record of all of the potentially-selectable files encountered.
    FileTable files;
    // A record of all of the potentially-selectable NamedSetOfFiles.
    FileSetTable file_sets;
    // Mapping from fileset id to target name which required that
    // file set when it had not yet been seen.
    absl::flat_hash_map<FileSetId, std::string> pending;
  };
  std::optional<BazelArtifact> SelectFileSet(
      absl::string_view id, const build_event_stream::NamedSetOfFiles& fileset);

  std::optional<BazelArtifact> SelectTargetCompleted(
      const build_event_stream::BuildEventId::TargetCompletedId& id,
      const build_event_stream::TargetComplete& payload);

  struct PartitionFileSetsResult {
    std::vector<FileSetId> selected;
    std::vector<FileSetId> unselected;
  };
  PartitionFileSetsResult PartitionFileSets(
      const build_event_stream::BuildEventId::TargetCompletedId& id,
      const build_event_stream::TargetComplete& payload);

  // Extracts the selected files into the (optional) `files` output.
  // If `files` is nullptr, extracted files will be dropped.
  void ExtractFilesInto(FileSetId id, absl::string_view target,
                        std::vector<BazelArtifactFile>* files);
  void InsertFileSet(FileSetId id,
                     const build_event_stream::NamedSetOfFiles& fileset);

  std::optional<FileSetId> InternUnlessDisposed(absl::string_view id) {
    return state_.file_sets.InternUnlessDisposed(id);
  }

  Options options_;
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
      absl::flat_hash_set<std::string> action_types = {});

  /// \brief Constructs an ExtraActionSelector from an allowlist pattern.
  /// Both a null and an empty pattern will match nothing.
  explicit ExtraActionSelector(const RE2* action_pattern);

  /// \brief Selects artifacts from ExtraAction-based extractors.
  std::optional<BazelArtifact> Select(
      const build_event_stream::BuildEvent& event) final;

 private:
  std::function<bool(absl::string_view)> action_matches_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_SELECTOR_H_
