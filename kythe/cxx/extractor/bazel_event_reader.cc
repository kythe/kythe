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
#include "kythe/cxx/extractor/bazel_event_reader.h"

#include <utility>

#include "absl/status/status.h"
#include "google/protobuf/util/delimited_message_util.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {
namespace {
using ::build_event_stream::BuildEvent;
using ::google::protobuf::io::CodedInputStream;
using ::google::protobuf::io::ZeroCopyInputStream;
using ::google::protobuf::util::ParseDelimitedFromCodedStream;
using ::google::protobuf::util::ParseDelimitedFromZeroCopyStream;

struct ParseEvent {
  BuildEvent* event;
  bool* clean_eof;

  template <typename T>
  bool From(T&& variant) {
    return absl::visit(*this, std::forward<T>(variant));
  }

  bool operator()(ZeroCopyInputStream* stream) const {
    return ParseDelimitedFromZeroCopyStream(event, stream, clean_eof);
  }

  bool operator()(CodedInputStream* stream) const {
    return ParseDelimitedFromCodedStream(event, stream, clean_eof);
  }
};

}  // namespace

void BazelEventReader::Next() {
  bool clean_eof = true;
  BuildEvent event;
  if (ParseEvent{&event, &clean_eof}.From(stream_)) {
    value_ = std::move(event);
  } else if (clean_eof) {
    value_ = absl::OkStatus();
  } else {
    value_ = absl::UnknownError("Error decoding BuildEvent from stream");
  }
}

}  // namespace kythe
