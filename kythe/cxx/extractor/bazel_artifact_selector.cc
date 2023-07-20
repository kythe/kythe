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

#include <cstdint>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/log/check.h"
#include "absl/log/die_if_null.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "google/protobuf/any.pb.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "kythe/proto/bazel_artifact_selector.pb.h"
#include "kythe/proto/bazel_artifact_selector_v2.pb.h"
#include "re2/re2.h"

namespace kythe {
namespace {

std::optional<std::string> ToUri(const build_event_stream::File& file) {
  switch (file.file_case()) {
    case build_event_stream::File::kUri:
      return file.uri();
    case build_event_stream::File::kContents:
      // We expect inline data to be rare and small, so always base64 encode it.
      return absl::StrCat(
          "data:base64,",
          // data URIs use regular base64, not "web safe" base64.
          absl::Base64Escape(file.contents()));
    case build_event_stream::File::kSymlinkTargetPath:
      return std::nullopt;
    default:
      break;
  }
  LOG(ERROR) << "Unexpected build_event_stream::File case!" << file.file_case();
  return std::nullopt;
}

std::string ToLocalPath(const build_event_stream::File& file) {
  std::vector<std::string> parts(file.path_prefix().begin(),
                                 file.path_prefix().end());
  parts.push_back(file.name());
  return absl::StrJoin(parts, "/");
}

std::optional<BazelArtifactFile> ToBazelArtifactFile(
    const build_event_stream::File& file, const RegexSet& allowlist) {
  if (!allowlist.Match(file.name())) {
    return std::nullopt;
  }
  std::optional<std::string> uri = ToUri(file);
  if (!uri.has_value()) return std::nullopt;
  return BazelArtifactFile{
      .local_path = ToLocalPath(file),
      .uri = *std::move(uri),
  };
}

template <typename T>
T& GetOrConstruct(std::optional<T>& value) {
  return value.has_value() ? *value : value.emplace();
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
bool StrictAtoI(absl::string_view value, int64_t* out) {
  if (value == "0") {
    *out = 0;
    return true;
  }
  if (value.empty() || value.front() == '0') {
    // We need to ignore leading zeros as they don't contribute to the integral
    // value.
    return false;
  }
  for (char ch : value) {
    if (!absl::ascii_isdigit(ch)) {
      return false;
    }
  }
  return absl::SimpleAtoi(value, out);
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

std::optional<BazelArtifact> AspectArtifactSelector::Select(
    const build_event_stream::BuildEvent& event) {
  std::optional<BazelArtifact> result = std::nullopt;
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

class AspectArtifactSelectorSerializationHelper {
 public:
  using FileId = AspectArtifactSelector::FileId;
  using ProtoFile = ::kythe::proto::BazelAspectArtifactSelectorStateV2::File;
  using FileSet = AspectArtifactSelector::FileSet;
  using ProtoFileSet =
      ::kythe::proto::BazelAspectArtifactSelectorStateV2::FileSet;
  using FileSetId = AspectArtifactSelector::FileSetId;
  using State = AspectArtifactSelector::State;

  static bool SerializeInto(
      const State& state,
      kythe::proto::BazelAspectArtifactSelectorStateV2& result) {
    return Serializer(&state, result).Serialize();
  }

  static absl::Status DeserializeFrom(
      const kythe::proto::BazelAspectArtifactSelectorStateV2& state,
      State& result) {
    return Deserializer(&state, result).Deserialize();
  }

 private:
  class Serializer {
   public:
    explicit Serializer(const State* state ABSL_ATTRIBUTE_LIFETIME_BOUND,
                        kythe::proto::BazelAspectArtifactSelectorStateV2& result
                            ABSL_ATTRIBUTE_LIFETIME_BOUND)
        : state_(*ABSL_DIE_IF_NULL(state)), result_(result) {}

    bool Serialize() {
      for (const auto& [id, file_set] : state_.file_sets.file_sets()) {
        SerializeFileSet(id, file_set);
      }
      for (FileSetId id : state_.file_sets.disposed()) {
        SerializeDisposed(id);
      }
      for (const auto& [id, target] : state_.pending) {
        SerializePending(id, target);
      }
      return true;
    }

   private:
    static int64_t ToSerializationId(FileSetId id, size_t other) {
      if (const auto [unpacked] = id; unpacked >= 0) {
        return unpacked;
      }
      // 0 is reserved for the integral ids, so start at -1.
      return -1 - static_cast<int64_t>(other);
    }

    int64_t SerializeFileSetId(FileSetId id) {
      auto [iter, inserted] = set_id_map_.try_emplace(
          id, ToSerializationId(id, result_.file_set_ids().size()));
      if (inserted && iter->second < 0) {
        result_.add_file_set_ids(state_.file_sets.ToString(id));
      }
      return iter->second;
    }

    void SerializeFileSet(FileSetId id, const FileSet& file_set) {
      auto& entry = (*result_.mutable_file_sets())[SerializeFileSetId(id)];
      for (FileId file_id : file_set.files) {
        if (std::optional<uint64_t> index = SerializeFile(file_id)) {
          entry.add_files(*index);
        }
      }
      for (FileSetId child_id : file_set.file_sets) {
        entry.add_file_sets(SerializeFileSetId(child_id));
      }
    }

    std::optional<uint64_t> SerializeFile(FileId id) {
      const BazelArtifactFile* file = state_.files.Find(id);
      if (file == nullptr) {
        LOG(INFO) << "Omitting extracted FileId from serialization: "
                  << std::get<0>(id);
        // FileSets may still reference files which have already been selected.
        // If so, don't keep them when serializing.
        return std::nullopt;
      }
      auto [iter, inserted] =
          file_id_map_.try_emplace(id, result_.files().size());
      if (!inserted) {
        return iter->second;
      }

      auto* entry = result_.add_files();
      entry->set_local_path(file->local_path);
      entry->set_uri(file->uri);
      return iter->second;
    }

    void SerializeDisposed(FileSetId id) {
      result_.add_disposed(SerializeFileSetId(id));
    }

    void SerializePending(FileSetId id, absl::string_view target) {
      (*result_.mutable_pending())[SerializeFileSetId(id)] = target;
    }

    const State& state_;
    kythe::proto::BazelAspectArtifactSelectorStateV2& result_;

    absl::flat_hash_map<FileId, uint64_t> file_id_map_;
    absl::flat_hash_map<FileSetId, int64_t> set_id_map_;
  };

  class Deserializer {
   public:
    explicit Deserializer(
        const kythe::proto::BazelAspectArtifactSelectorStateV2* state
            ABSL_ATTRIBUTE_LIFETIME_BOUND,
        State& result ABSL_ATTRIBUTE_LIFETIME_BOUND)
        : state_(*ABSL_DIE_IF_NULL(state)), result_(result) {}

    absl::Status Deserialize() {
      // First, deserialize all of the disposed sets to help check consistency
      // during the rest of deserialization.
      for (int64_t id : state_.disposed()) {
        absl::StatusOr<FileSetId> real_id = DeserializeFileSetId(id);
        if (!real_id.ok()) return real_id.status();
        result_.file_sets.Dispose(*real_id);
      }
      {
        // Then check the file_set_ids list for uniqueness:
        absl::flat_hash_set<std::string> non_integer_ids(
            state_.file_set_ids().begin(), state_.file_set_ids().end());
        if (non_integer_ids.size() != state_.file_set_ids().size()) {
          return absl::InvalidArgumentError("Inconsistent file_set_ids map");
        }
      }

      for (const auto& [id, file_set] : state_.file_sets()) {
        // Ensure pending and live file sets are distinct.
        if (state_.pending().contains(id)) {
          return absl::InvalidArgumentError(
              absl::StrCat("FileSet ", id, " is both pending and live"));
        }
        absl::Status status = DeserializeFileSet(id, file_set);
        if (!status.ok()) return status;
      }
      for (const auto& [id, target] : state_.pending()) {
        absl::Status status = DeserializePending(id, target);
        if (!status.ok()) return status;
      }
      return absl::OkStatus();
    }

   private:
    static constexpr FileSetId kDummy{0};

    static absl::StatusOr<std::string> ToDeserializationId(
        const kythe::proto::BazelAspectArtifactSelectorStateV2& state,
        int64_t id) {
      if (id < 0) {
        // Normalize the -1 based index.
        size_t index = -(id + 1);
        if (index > state.file_set_ids().size()) {
          return absl::InvalidArgumentError(absl::StrCat(
              "Non-integral FileSetId index out of range: ", index));
        }
        return state.file_set_ids(index);
      }
      return absl::StrCat(id);
    }

    absl::StatusOr<FileSetId> DeserializeFileSetId(int64_t id) {
      auto [iter, inserted] = set_id_map_.try_emplace(id, kDummy);
      if (inserted) {
        absl::StatusOr<std::string> string_id = ToDeserializationId(state_, id);
        if (!string_id.ok()) return string_id.status();

        std::optional<FileSetId> file_set_id =
            result_.file_sets.InternUnlessDisposed(*string_id);
        if (!file_set_id.has_value()) {
          return absl::InvalidArgumentError(
              "Encountered disposed FileSetId during deserialization");
        }
        iter->second = *file_set_id;
      }
      return iter->second;
    }

    absl::Status DeserializeFileSet(int64_t id, const ProtoFileSet& file_set) {
      absl::StatusOr<FileSetId> file_set_id = DeserializeFileSetId(id);
      if (!file_set_id.ok()) return file_set_id.status();

      FileSet result_set;
      for (uint64_t file_id : file_set.files()) {
        absl::StatusOr<FileId> real_id = DeserializeFile(file_id);
        if (!real_id.ok()) return real_id.status();

        result_set.files.push_back(*real_id);
      }
      for (int64_t child_id : file_set.file_sets()) {
        if (!(state_.file_sets().contains(child_id) ||
              state_.pending().contains(child_id))) {
          // Ensure internal consistency.
          return absl::InvalidArgumentError(absl::StrCat(
              "Child FileSetId is neither live nor pending: ", id));
        }

        absl::StatusOr<FileSetId> real_id = DeserializeFileSetId(child_id);
        if (!real_id.ok()) return real_id.status();

        result_set.file_sets.push_back(*real_id);
      }
      if (!result_.file_sets.InsertUnlessDisposed(*file_set_id,
                                                  std::move(result_set))) {
        return absl::InvalidArgumentError(
            absl::StrCat("FileSetId both disposed and live: ", id));
      }
      return absl::OkStatus();
    }

    absl::StatusOr<FileId> DeserializeFile(uint64_t id) {
      if (id > state_.files_size()) {
        return absl::InvalidArgumentError(
            absl::StrCat("File index out of range: ", id));
      }
      return result_.files.Insert(BazelArtifactFile{
          .local_path = state_.files(id).local_path(),
          .uri = state_.files(id).uri(),
      });
    }

    absl::Status DeserializePending(int64_t id, absl::string_view target) {
      absl::StatusOr<FileSetId> real_id = DeserializeFileSetId(id);
      if (!real_id.ok()) return real_id.status();

      result_.pending.try_emplace(*real_id, target);
      return absl::OkStatus();
    }

    const kythe::proto::BazelAspectArtifactSelectorStateV2& state_;
    State& result_;

    absl::flat_hash_map<int64_t, FileSetId> set_id_map_;
  };
};

bool AspectArtifactSelector::SerializeInto(google::protobuf::Any& state) const {
  switch (options_.serialization_format) {
    case AspectArtifactSelectorSerializationFormat::kV2: {
      kythe::proto::BazelAspectArtifactSelectorStateV2 raw;
      if (!AspectArtifactSelectorSerializationHelper::SerializeInto(state_,
                                                                    raw)) {
        return false;
      }
      state.PackFrom(std::move(raw));
      return true;
    }
    case AspectArtifactSelectorSerializationFormat::kV1: {
      kythe::proto::BazelAspectArtifactSelectorState raw;
      for (FileSetId id : state_.file_sets.disposed()) {
        raw.add_disposed(state_.file_sets.ToString(id));
      }
      for (const auto& [id, target] : state_.pending) {
        (*raw.mutable_pending())[state_.file_sets.ToString(id)] = target;
      }
      for (const auto& [id, file_set] : state_.file_sets.file_sets()) {
        auto& entry = (*raw.mutable_filesets())[state_.file_sets.ToString(id)];
        for (FileSetId child_id : file_set.file_sets) {
          entry.add_file_sets()->set_id(state_.file_sets.ToString(child_id));
        }
        for (FileId file_id : file_set.files) {
          const BazelArtifactFile* file = state_.files.Find(file_id);
          if (file == nullptr) continue;

          auto* file_entry = entry.add_files();
          file_entry->set_name(file->local_path);
          file_entry->set_uri(file->uri);
        }
      }
      state.PackFrom(std::move(raw));
      return true;
    }
  }
  return false;
}

absl::Status AspectArtifactSelector::DeserializeFrom(
    const google::protobuf::Any& state) {
  if (auto raw = kythe::proto::BazelAspectArtifactSelectorStateV2();
      state.UnpackTo(&raw)) {
    state_ = {};
    return AspectArtifactSelectorSerializationHelper::DeserializeFrom(raw,
                                                                      state_);
  } else if (state.Is<kythe::proto::BazelAspectArtifactSelectorStateV2>()) {
    return absl::InvalidArgumentError(
        "Malformed kythe.proto.BazelAspectArtifactSelectorStateV2");
  }
  if (auto raw = kythe::proto::BazelAspectArtifactSelectorState();
      state.UnpackTo(&raw)) {
    state_ = {};
    for (const auto& id : raw.disposed()) {
      if (std::optional<FileSetId> file_set_id =
              state_.file_sets.InternUnlessDisposed(id)) {
        state_.file_sets.Dispose(*file_set_id);
      }
    }
    for (const auto& [id, target] : raw.pending()) {
      if (std::optional<FileSetId> file_set_id =
              state_.file_sets.InternUnlessDisposed(id)) {
        state_.pending.try_emplace(*file_set_id, target);
      }
    }
    for (const auto& [id, file_set] : raw.filesets()) {
      if (std::optional<FileSetId> file_set_id =
              state_.file_sets.InternUnlessDisposed(id)) {
        InsertFileSet(*file_set_id, file_set);
      }
    }
    return absl::OkStatus();
  } else if (state.Is<kythe::proto::BazelAspectArtifactSelectorState>()) {
    return absl::InvalidArgumentError(
        "Malformed kythe.proto.BazelAspectArtifactSelectorState");
  }
  return absl::FailedPreconditionError(
      "State not of type kythe.proto.BazelAspectArtifactSelectorState");
}

AspectArtifactSelector::FileTable::FileTable(const FileTable& other)
    : next_id_(other.next_id_),
      file_map_(other.file_map_),
      id_map_(file_map_.size()) {
  for (const auto& [file, entry] : file_map_) {
    id_map_.insert_or_assign(entry.id, &file);
  }
}

AspectArtifactSelector::FileTable& AspectArtifactSelector::FileTable::operator=(
    const FileTable& other) {
  next_id_ = other.next_id_;
  file_map_ = other.file_map_;
  id_map_.clear();
  for (const auto& [file, entry] : file_map_) {
    id_map_.insert_or_assign(entry.id, &file);
  }
  return *this;
}

AspectArtifactSelector::FileId AspectArtifactSelector::FileTable::Insert(
    BazelArtifactFile file) {
  auto [iter, inserted] = file_map_.emplace(
      std::move(file), Entry{.id = FileId(next_id_), .count = 1});
  if (inserted) {
    next_id_++;
    id_map_[iter->second.id] = &iter->first;
  } else {
    iter->second.count++;
  }
  return iter->second.id;
}

BazelArtifactFile AspectArtifactSelector::FileTable::ExtractIterators(
    IdMap::iterator id_iter, FileMap::iterator file_iter) {
  CHECK(id_iter != id_map_.end());
  CHECK(file_iter != file_map_.end());
  if (--file_iter->second.count == 0) {
    // Only remove the file once it's been extracted for each FileSet which
    // references it.
    id_map_.erase(id_iter);
    return std::move(file_map_.extract(file_iter).key());
  }
  return file_iter->first;
}

std::optional<BazelArtifactFile> AspectArtifactSelector::FileTable::Extract(
    FileId id) {
  auto id_iter = id_map_.find(id);
  if (id_iter == id_map_.end()) {
    return std::nullopt;
  }
  // file_map_ owns the memory underlying the pointer we dereferenced here.
  // If it's missing from the map, we're well into UB trouble.
  return ExtractIterators(id_iter, file_map_.find(*id_iter->second));
}

BazelArtifactFile AspectArtifactSelector::FileTable::ExtractFile(
    BazelArtifactFile file) {
  auto file_iter = file_map_.find(file);
  if (file_iter == file_map_.end()) {
    return file;
  }
  // If the file id is missing from id_map_, something has gone horribly wrong
  // with our invariants.
  return ExtractIterators(id_map_.find(file_iter->second.id), file_iter);
}

const BazelArtifactFile* AspectArtifactSelector::FileTable::Find(
    FileId id) const {
  auto iter = id_map_.find(id);
  if (iter == id_map_.end()) {
    return nullptr;
  }
  return iter->second;
}

std::optional<AspectArtifactSelector::FileSetId>
AspectArtifactSelector::FileSetTable::InternUnlessDisposed(
    absl::string_view id) {
  auto [result, inserted] = InternOrCreate(id);
  if (!inserted && disposed_.contains(result)) {
    return std::nullopt;
  }
  return result;
}

std::pair<AspectArtifactSelector::FileSetId, bool>
AspectArtifactSelector::FileSetTable::InternOrCreate(absl::string_view id) {
  int64_t token;
  if (StrictAtoI(id, &token)) {
    return {{token}, false};
  }
  auto [iter, inserted] = id_map_.try_emplace(id, std::make_tuple(next_id_));
  if (inserted) {
    next_id_--;  // Non-integral ids are mapped to negative values.
    inverse_id_map_.try_emplace(iter->second, iter->first);
  }
  return {{iter->second}, inserted};
}

bool AspectArtifactSelector::FileSetTable::InsertUnlessDisposed(
    FileSetId id, FileSet file_set) {
  if (disposed_.contains(id)) {
    return false;
  }
  file_sets_.insert_or_assign(id, std::move(file_set));
  return true;  // A false return indicates the set has already been disposed.
}

std::optional<AspectArtifactSelector::FileSet>
AspectArtifactSelector::FileSetTable::ExtractAndDispose(FileSetId id) {
  if (auto node = file_sets_.extract(id); !node.empty()) {
    disposed_.insert(id);
    return std::move(node.mapped());
  }
  return std::nullopt;
}

void AspectArtifactSelector::FileSetTable::Dispose(FileSetId id) {
  disposed_.insert(id);
  file_sets_.erase(id);
}

bool AspectArtifactSelector::FileSetTable::Disposed(FileSetId id) {
  return disposed_.contains(id);
}

std::string AspectArtifactSelector::FileSetTable::ToString(FileSetId id) const {
  if (const auto [unpacked] = id; unpacked >= 0) {
    return absl::StrCat(unpacked);
  }
  return inverse_id_map_.at(id);
}

std::optional<BazelArtifact> AspectArtifactSelector::SelectFileSet(
    absl::string_view id, const build_event_stream::NamedSetOfFiles& fileset) {
  std::optional<FileSetId> file_set_id = InternUnlessDisposed(id);
  if (!file_set_id.has_value()) {
    // Already disposed, skip.
    return std::nullopt;
  }
  // This was a pending file set, select it directly.
  if (auto node = state_.pending.extract(*file_set_id); !node.empty()) {
    state_.file_sets.Dispose(*file_set_id);
    BazelArtifact result = {.label = node.mapped()};
    for (const auto& file : fileset.files()) {
      if (std::optional<BazelArtifactFile> artifact_file =
              ToBazelArtifactFile(file, options_.file_name_allowlist)) {
        result.files.push_back(
            state_.files.ExtractFile(*std::move(artifact_file)));
      }
    }
    for (const auto& child : fileset.file_sets()) {
      if (std::optional<FileSetId> child_id =
              InternUnlessDisposed(child.id())) {
        ExtractFilesInto(*child_id, result.label, &result.files);
      }
    }
    return result;
  }
  InsertFileSet(*file_set_id, fileset);
  return std::nullopt;
}

std::optional<BazelArtifact> AspectArtifactSelector::SelectTargetCompleted(
    const build_event_stream::BuildEventId::TargetCompletedId& id,
    const build_event_stream::TargetComplete& payload) {
  BazelArtifact result = {
      .label = id.label(),
  };
  const auto& [selected, unselected] = PartitionFileSets(id, payload);
  for (FileSetId file_set_id : selected) {
    ExtractFilesInto(file_set_id, result.label, &result.files);
  }
  if (options_.dispose_unselected_output_groups) {
    for (FileSetId file_set_id : unselected) {
      ExtractFilesInto(file_set_id, result.label, nullptr);
    }
  }
  if (!result.files.empty()) {
    return result;
  }
  return std::nullopt;
}

AspectArtifactSelector::PartitionFileSetsResult
AspectArtifactSelector::PartitionFileSets(
    const build_event_stream::BuildEventId::TargetCompletedId& id,
    const build_event_stream::TargetComplete& payload) {
  PartitionFileSetsResult result;
  bool id_match = options_.target_aspect_allowlist.Match(id.aspect());
  for (const auto& output_group : payload.output_group()) {
    auto& output =
        (id_match && options_.output_group_allowlist.Match(output_group.name()))
            ? result.selected
            : result.unselected;
    for (const auto& fileset : output_group.file_sets()) {
      if (std::optional<FileSetId> file_set_id =
              InternUnlessDisposed(fileset.id())) {
        output.push_back(*file_set_id);
      }
    }
  }
  return result;
}

void AspectArtifactSelector::ExtractFilesInto(
    FileSetId id, absl::string_view target,
    std::vector<BazelArtifactFile>* files) {
  if (state_.file_sets.Disposed(id)) {
    return;
  }

  std::optional<FileSet> file_set = state_.file_sets.ExtractAndDispose(id);
  if (!file_set.has_value()) {
    // Files where requested, but we haven't disposed that filesets id yet.
    // Record this for future processing.
    LOG(INFO) << "NamedSetOfFiles " << state_.file_sets.ToString(id)
              << " requested by " << target << " but not yet disposed.";
    if (files != nullptr) {
      // Only retain pending file sets if they would've been saved.
      state_.pending.emplace(id, target);
    } else if (state_.pending.find(id) == state_.pending.end()) {
      // But still prefer to retain pending file sets.
      state_.file_sets.Dispose(id);
    }
    return;
  }

  for (FileId file_id : file_set->files) {
    if (std::optional<BazelArtifactFile> file = state_.files.Extract(file_id);
        file.has_value() && files != nullptr) {
      files->push_back(*std::move(file));
    }
  }
  for (FileSetId child_id : file_set->file_sets) {
    ExtractFilesInto(child_id, target, files);
  }
}

void AspectArtifactSelector::InsertFileSet(
    FileSetId id, const build_event_stream::NamedSetOfFiles& fileset) {
  std::optional<FileSet> file_set;
  for (const auto& file : fileset.files()) {
    if (std::optional<BazelArtifactFile> artifact_file =
            ToBazelArtifactFile(file, options_.file_name_allowlist)) {
      FileId file_id = state_.files.Insert(*std::move(artifact_file));
      GetOrConstruct(file_set).files.push_back(file_id);
    }
  }
  for (const auto& child : fileset.file_sets()) {
    if (std::optional<FileSetId> child_id = InternUnlessDisposed(child.id())) {
      GetOrConstruct(file_set).file_sets.push_back(*child_id);
    }
  }
  if (file_set.has_value()) {
    state_.file_sets.InsertUnlessDisposed(id, *std::move(file_set));
  } else {
    // Nothing to do with this fileset, mark it disposed.
    state_.file_sets.Dispose(id);
  }
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

std::optional<BazelArtifact> ExtraActionSelector::Select(
    const build_event_stream::BuildEvent& event) {
  if (event.id().has_action_completed() && event.action().success() &&
      action_matches_(event.action().type())) {
    if (std::optional<std::string> uri =
            ToUri(event.action().primary_output())) {
      return BazelArtifact{
          .label = event.id().action_completed().label(),
          .files = {{
              .local_path = event.id().action_completed().primary_output(),
              .uri = *std::move(uri),
          }},
      };
    }
  }
  return std::nullopt;
}

}  // namespace kythe
