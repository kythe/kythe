/*
 * Copyright 2014 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_INDEXING_RECORDING_OUTPUT_STREAM_H_
#define KYTHE_CXX_COMMON_INDEXING_RECORDING_OUTPUT_STREAM_H_

#include "KytheOutputStream.h"

namespace kythe {

/// \brief A `KytheOutputStream` that records all `Entry` instances in memory.
///
/// This is intended to be used for testing only.
class RecordingOutputStream : public KytheOutputStream {
 public:
  void Emit(const FactRef &fact) override {
    proto::Entry entry;
    fact.Expand(&entry);
    Emit(entry);
  }
  void Emit(const EdgeRef &edge) override {
    proto::Entry entry;
    edge.Expand(&entry);
    entry.set_fact_name("/");
    Emit(entry);
  }
  void Emit(const OrdinalEdgeRef &edge) override {
    proto::Entry entry;
    edge.Expand(&entry);
    entry.set_fact_name("/");
    Emit(entry);
  }

  /// \brief All entries that were emitted to this stream, in order.
  const std::vector<kythe::proto::Entry> &entries() const { return entries_; }

 private:
  std::vector<kythe::proto::Entry> entries_;

  /// \brief Append an entry to this stream's history.
  void Emit(const kythe::proto::Entry &entry) { entries_.push_back(entry); }
};

}  // namespace kythe

#endif  // KYTHE_CXX_COMMON_INDEXING_RECORDING_OUTPUT_STREAM_H_
