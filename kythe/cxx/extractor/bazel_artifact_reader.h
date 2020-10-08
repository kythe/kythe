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
#ifndef KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_READER_H_
#define KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_READER_H_

#include <functional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "glog/logging.h"
#include "kythe/cxx/extractor/bazel_event_reader.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

/// \brief A callback which will be invoked on each BuildEvent in turn.  If the
/// selector can supply a BazelArtifact as a result of this event, it should
/// do so via the second parameter and return true.
/// A return value of false indicates no match and the artifacts should be
/// unmodified.
using ArtifactSelector =
    std::function<bool(const build_event_stream::BuildEvent&, BazelArtifact*)>;

class BazelArtifactReader {
 public:
  using value_type = BazelArtifact;
  using reference = value_type&;
  using const_reference = const value_type&;

  static std::vector<ArtifactSelector> DefaultSelectors();

  explicit BazelArtifactReader(
      BazelEventReaderInterface* event_reader,
      std::vector<ArtifactSelector> selectors = DefaultSelectors())
      : event_reader_(CHECK_NOTNULL(event_reader)),
        selectors_(std::move(selectors)) {
    Select();
  }

  void Next();

  bool Done() const { return value_.index() != 0; }
  reference Ref() { return absl::get<value_type>(value_); }
  const_reference Ref() const { return absl::get<value_type>(value_); }
  absl::Status status() const { return absl::get<absl::Status>(value_); }

 private:
  void Select();

  absl::variant<value_type, absl::Status> value_;
  BazelEventReaderInterface* event_reader_;
  std::vector<ArtifactSelector> selectors_;
};

/// \brief An ArtifactSelector which selects artifacts emitted by extra
/// actions.
///
/// This will select any successful ActionCompleted build event, but the
/// selection can be restricted to an allowlist of action_types.
struct ExtraActionSelector {
  /// \brief Allowlist against which to match ActionCompleted events.
  /// An empty set will select any successful action.
  absl::flat_hash_set<std::string> action_types;

  /// \brief Sets result from the primary output of a successful
  /// ActionCompleted event.
  bool operator()(const build_event_stream::BuildEvent& event,
                  BazelArtifact* result) const;
};

/// \brief An ArtifactSelector which selects artifacts by output group.
///
/// This is a stateful selector as it needs to track the named sets of files
/// from the output stream.
class OutputGroupSelector {
 public:
  explicit OutputGroupSelector(absl::flat_hash_set<std::string> group_names)
      : group_names_(std::move(group_names)) {}

  bool operator()(const build_event_stream::BuildEvent& event,
                  BazelArtifact* result);

 private:
  void FindNamedFiles(std::vector<std::string> ids,
                      std::vector<BazelArtifactFile>* result);

  absl::flat_hash_set<std::string> group_names_;
  absl::flat_hash_map<std::string, build_event_stream::NamedSetOfFiles>
      filesets_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_READER_H_
