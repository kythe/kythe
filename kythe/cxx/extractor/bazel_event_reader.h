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
#ifndef KYTHE_CXX_EXTRACTOR_BAZEL_EVENT_READER_H_
#define KYTHE_CXX_EXTRACTOR_BAZEL_EVENT_READER_H_

#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "glog/logging.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

/// \brief Reads BazelEvent messages from a binary Bazel event stream.
class BazelEventReader {
 public:
  using value_type = ::build_event_stream::BuildEvent;
  using reference = value_type&;
  using const_reference = const value_type&;

  /// \brief Constructs a new BazelEventReader
  explicit BazelEventReader(google::protobuf::io::CodedInputStream* stream)
      : stream_(stream) {
    Next();
  }

  /// \brief Constructs a new BazelEventReader
  explicit BazelEventReader(google::protobuf::io::ZeroCopyInputStream* stream)
      : stream_(stream) {
    Next();
  }

  void Next();

  bool Done() const { return value_.index() != 0; }
  reference Ref() { return absl::get<value_type>(value_); }
  const_reference Ref() const { return absl::get<value_type>(value_); }
  absl::Status status() const { return absl::get<absl::Status>(value_); }

 private:
  absl::variant<value_type, absl::Status> value_;
  absl::variant<google::protobuf::io::CodedInputStream*,
                google::protobuf::io::ZeroCopyInputStream*>
      stream_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_EVENT_READER_H_
