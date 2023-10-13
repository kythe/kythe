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
#include "absl/log/die_if_null.h"
#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "kythe/cxx/extractor/bazel_artifact.h"
#include "kythe/cxx/extractor/bazel_artifact_selector.h"
#include "kythe/cxx/extractor/bazel_event_reader.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

/// \brief A cursor which selects and filters extraction artifacts from an
/// underlying Bazel BuildEvent stream.
class BazelArtifactReader {
 public:
  using value_type = BazelArtifact;
  using reference = value_type&;
  using const_reference = const value_type&;

  /// \brief Returns a list of default BazelArtifactSelectors consisting of a
  /// default-constructed ExtraActionSelector.
  static std::vector<AnyArtifactSelector> DefaultSelectors();

  /// \brief Constructs a new BazelArtifactSelector over the provided event
  /// stream.
  explicit BazelArtifactReader(
      BazelEventReaderInterface* event_reader,
      std::vector<AnyArtifactSelector> selectors = DefaultSelectors())
      : event_reader_(ABSL_DIE_IF_NULL(event_reader)),
        selectors_(std::move(selectors)) {
    Select();
  }

  /// \brief Advances the event stream until the next artifact is found.
  /// Invokes all selectors on sequential events from the stream until at least
  /// one selector signals success.
  void Next();

  /// \brief Returns true if the underlying stream is exahusted and there are
  /// no more artifacts to find.
  bool Done() const { return value_.index() != 0; }

  /// \brief Returns a reference to the most recently selected artifact.
  /// Requires: Done() == false.
  reference Ref() { return absl::get<value_type>(value_); }
  const_reference Ref() const { return absl::get<value_type>(value_); }

  /// \brief Returns the overall status of the stream.
  /// Requires: Done() == true.
  absl::Status status() const { return absl::get<absl::Status>(value_); }

 private:
  void Select();

  absl::variant<value_type, absl::Status> value_;
  BazelEventReaderInterface* event_reader_;
  std::vector<AnyArtifactSelector> selectors_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_READER_H_
