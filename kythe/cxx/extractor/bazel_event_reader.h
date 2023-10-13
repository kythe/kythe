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

#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

namespace kythe {

/// \brief Interface used for iterating through Bazel BuildEvent messages.
class BazelEventReaderInterface {
 public:
  using value_type = ::build_event_stream::BuildEvent;
  using reference = value_type&;
  using const_reference = const value_type&;

  /// \brief Advances the reading to the next record.
  virtual void Next() = 0;
  /// \brief Returns false if there are no more records to read.
  virtual bool Done() const = 0;
  /// \brief Returns a reference to the current record.
  /// Requires: !Done()
  virtual reference Ref() = 0;
  /// \brief Returns a const-reference to the current record.
  /// Requires: !Done()
  virtual const_reference Ref() const = 0;
  /// \brief Returns the overall status of the iteration.
  virtual absl::Status status() const = 0;

  virtual ~BazelEventReaderInterface() = default;

 protected:
  /// \brief While BazelEventReaderInterface is not itself copyable, subclasses
  /// choose to be.
  BazelEventReaderInterface() = default;
  BazelEventReaderInterface(const BazelEventReaderInterface&) = default;
  BazelEventReaderInterface& operator=(const BazelEventReaderInterface&) =
      default;
};

/// \brief Reads BazelEvent messages from a binary Bazel event stream.
class BazelEventReader final : public BazelEventReaderInterface {
 public:
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

  void Next() override;

  bool Done() const override { return value_.index() != 0; }
  reference Ref() override { return absl::get<value_type>(value_); }
  const_reference Ref() const override { return absl::get<value_type>(value_); }
  absl::Status status() const override {
    return absl::get<absl::Status>(value_);
  }

 private:
  absl::variant<value_type, absl::Status> value_;
  absl::variant<google::protobuf::io::CodedInputStream*,
                google::protobuf::io::ZeroCopyInputStream*>
      stream_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_EXTRACTOR_BAZEL_EVENT_READER_H_
