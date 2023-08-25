/*
 * Copyright 2014 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/indexing/KytheCachingOutput.h"

#include <cassert>
#include <cstddef>
#include <cstdio>
#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "google/protobuf/io/coded_stream.h"
#include "kythe/proto/storage.pb.h"

namespace kythe {

std::string FileOutputStream::Stats::ToString() const {
  return absl::StrCat(
      buffers_merged_, " merged ", buffers_split_, " split ", buffers_retired_,
      " retired ", hashes_matched_, " matches ",
      (buffers_retired_ ? (total_bytes_ / buffers_retired_) : 0),
      " bytes/buffer");
}

FileOutputStream::~FileOutputStream() {
  while (!buffers_.empty()) {
    // Shake out any less-than-minimum-sized buffers that remain.
    EmitAndReleaseTopBuffer();
  }
  if (show_stats_) {
    absl::FPrintF(stderr, "%s\n", stats_.ToString());
    fflush(stderr);
  }
}

void FileOutputStream::EnqueueEntry(const proto::Entry& entry) {
  if (cache_ == &default_cache_ || buffers_.empty()) {
    {
      google::protobuf::io::CodedOutputStream coded_stream(stream_);
      coded_stream.WriteVarint32(entry.ByteSizeLong());
      entry.SerializeToCodedStream(&coded_stream);
    }
    if (flush_after_each_entry_) {
      stream_->Flush();
    }
    return;
  }

  size_t entry_size = entry.ByteSizeLong();
  size_t size_size =
      google::protobuf::io::CodedOutputStream::VarintSize32(entry_size);
  size_t size_delta = entry_size + size_size;
  unsigned char* buffer = buffers_.WriteToTop(size_delta);

  google::protobuf::io::CodedOutputStream::WriteVarint32ToArray(entry_size,
                                                                buffer);
  if (!entry.SerializeToArray(&buffer[size_size], size_delta - size_size)) {
    assert(0 && "bad proto size calculation");
  }
  stats_.total_bytes_ += size_delta;

  if (buffers_.top_size() >= max_size_) {
    ++stats_.buffers_split_;
    EmitAndReleaseTopBuffer();
    PushBuffer();
  }
}

void FileOutputStream::EmitAndReleaseTopBuffer() {
  HashCache::Hash hash;
  buffers_.HashTop(&hash);
  if (!cache_->SawHash(hash)) {
    buffers_.CopyTopToStream(stream_);
    if (flush_after_each_entry_) {
      stream_->Flush();
    }
    cache_->RegisterHash(hash);
  } else {
    ++stats_.hashes_matched_;
  }
  buffers_.Pop();
  ++stats_.buffers_retired_;
}

void FileOutputStream::PushBuffer() { buffers_.Push(max_size_); }

void FileOutputStream::PopBuffer() {
  if (buffers_.MergeDownIfTooSmall(min_size_, max_size_)) {
    ++stats_.buffers_merged_;
  } else {
    EmitAndReleaseTopBuffer();
  }
}

}  // namespace kythe
