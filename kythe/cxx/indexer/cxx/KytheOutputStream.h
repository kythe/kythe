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

#ifndef KYTHE_CXX_INDEXER_CXX_KYTHE_OUTPUT_STREAM_H_
#define KYTHE_CXX_INDEXER_CXX_KYTHE_OUTPUT_STREAM_H_

#include <memory>
#include <vector>

#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include "gflags/gflags.h"
#include "kythe/proto/storage.pb.h"

DECLARE_bool(flush_after_each_entry);

namespace kythe {

// Interface for receiving Kythe `Entity` instances.
// TODO(jdennett): This interface should be recast in terms of emitting facts
//                 instead of directly constructing `Entity` protos.
//                 (Also see KytheGraphObserver.cc.)
class KytheOutputStream {
 public:
  virtual void Emit(const kythe::proto::Entry &entry) = 0;
};

// A `KytheOutputStream` that records `Entry` instances to a
// `FileOutputStream`.
class FileOutputStream : public KytheOutputStream {
 public:
  FileOutputStream(google::protobuf::io::FileOutputStream *stream)
      : stream_(stream) {}

  void Emit(const kythe::proto::Entry &entry) override {
    {
      google::protobuf::io::CodedOutputStream coded_stream(stream_);
      coded_stream.WriteVarint32(entry.ByteSize());
      entry.SerializeToCodedStream(&coded_stream);
    }
    if (FLAGS_flush_after_each_entry) {
      stream_->Flush();
    }
  }

 private:
  google::protobuf::io::FileOutputStream *stream_;
};

}  // namespace kythe

#endif  // KYTHE_CXX_INDEXER_CXX_KYTHE_OUTPUT_STREAM_H_
