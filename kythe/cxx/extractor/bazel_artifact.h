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

#ifndef KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_H_
#define KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_H_

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"

namespace kythe {

/// \brief A pair of local path and canonical URI for a given Bazel output file.
/// Bazel typically provides both a workspace-local path to a file as well as a
/// canonical URI and both can be useful to clients.
struct BazelArtifactFile {
  std::string local_path;  ///< The workspace-relative path to the file.
  std::string uri;  ///< The canonical URI for the file. Clients should be able
                    ///< to handle at least `file:` and `data:` schemes.

  bool operator==(const BazelArtifactFile& other) const {
    return std::tie(local_path, uri) == std::tie(other.local_path, other.uri);
  }

  bool operator!=(const BazelArtifactFile& other) const {
    return !(*this == other);
  }

  template <typename H>
  friend H AbslHashValue(H h, const BazelArtifactFile& file) {
    return H::combine(std::move(h), file.local_path, file.uri);
  }

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const BazelArtifactFile& file) {
    absl::Format(&sink, "BazelArtifactFile{.local_path = %v, .uri = %v}",
                 absl::FormatStreamed(std::quoted(file.local_path)),
                 absl::FormatStreamed(std::quoted(file.uri)));
  }
  friend std::ostream& operator<<(std::ostream& out,
                                  const BazelArtifactFile& file) {
    return (out << absl::StreamFormat("%v", file));
  }
};

/// \brief A list of extracted compilation units and the target which owns them.
struct BazelArtifact {
  std::string label;  ///< The target from which these artifacts originate.
  std::vector<BazelArtifactFile>
      files;  ///< A list of paths and URIs at which the artifacts can be found.

  bool operator==(const BazelArtifact& other) const {
    return std::tie(label, files) == std::tie(other.label, other.files);
  }

  bool operator!=(const BazelArtifact& other) const {
    return !(*this == other);
  }

  template <typename H>
  friend H AbslHashValue(H h, const BazelArtifact& artifact) {
    return H::combine(std::move(h), artifact.label, artifact.files);
  }

  template <typename Sink>
  friend void AbslStringify(Sink& sink, const BazelArtifact& artifact) {
    absl::Format(&sink, "BazelArtifact{.label = %v, .files = { %s }}",
                 absl::FormatStreamed(std::quoted(artifact.label)),
                 absl::StrJoin(artifact.files, ", "));
  }
  friend std::ostream& operator<<(std::ostream& out,
                                  const BazelArtifact& artifact) {
    return (out << absl::StreamFormat("%v", artifact));
  }
};

}  // namespace kythe
#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_ARTIFACT_H_
