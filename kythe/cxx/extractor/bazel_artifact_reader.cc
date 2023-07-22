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
#include "absl/strings/str_join.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

void BazelArtifactReader::Next() {
  if (event_reader_->Done()) return;
  event_reader_->Next();
  Select();
}

void BazelArtifactReader::Select() {
  for (; !event_reader_->Done(); event_reader_->Next()) {
    const auto& event = event_reader_->Ref();
    for (auto& selector : selectors_) {
      if (std::optional<BazelArtifact> artifact = selector.Select(event)) {
        value_ = *std::move(artifact);
        return;
      }
    }
  }
  // Both we and the underlying reader are done; propagate the status.
  value_ = event_reader_->status();
}

/* static */
std::vector<AnyArtifactSelector> BazelArtifactReader::DefaultSelectors() {
  return {ExtraActionSelector{}};
}
}  // namespace kythe
