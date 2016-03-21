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

#include "KytheOutputStream.h"

#include <algorithm>

#include <libmemcached/memcached.h>

#include "llvm/Support/raw_ostream.h"

namespace kythe {

bool MemcachedHashCache::OpenMemcache(const std::string &spec) {
  if (cache_) {
    memcached_free(cache_);
    cache_ = nullptr;
  }
  std::string spec_amend = spec;
  spec_amend.append(" --BINARY-PROTOCOL");
  cache_ = memcached(spec_amend.c_str(), spec_amend.size());
  if (cache_ != nullptr) {
    memcached_return_t remote_version = memcached_version(cache_);
    return memcached_success(remote_version);
  }
  return false;
}

MemcachedHashCache::~MemcachedHashCache() {
  if (cache_) {
    memcached_free(cache_);
    cache_ = nullptr;
  }
}

void MemcachedHashCache::RegisterHash(const Hash &hash) {
  if (!cache_) {
    return;
  }
  char value = 1;
  memcached_return_t add_result =
      memcached_add(cache_, reinterpret_cast<const char *>(hash), kHashSize,
                    &value, sizeof(value), 0, 0);
  if (!memcached_success(add_result) && add_result != MEMCACHED_DATA_EXISTS) {
    fprintf(stderr, "memcached add failed: %s\n",
            memcached_strerror(cache_, add_result));
  }
}

bool MemcachedHashCache::SawHash(const Hash &hash) {
  if (!cache_) {
    return false;
  }
  memcached_return_t ex_result =
      memcached_exist(cache_, reinterpret_cast<const char *>(hash), kHashSize);
  if (ex_result == MEMCACHED_SUCCESS) {
    return true;
  } else if (ex_result != MEMCACHED_NOTFOUND) {
    fprintf(stderr, "memcached exist failed: %s\n",
            memcached_strerror(cache_, ex_result));
  }
  return false;
}

std::string FileOutputStream::Stats::ToString() const {
  std::string out;
  llvm::raw_string_ostream ostream(out);
  ostream << buffers_merged_ << " merged " << buffers_split_ << " split "
          << buffers_retired_ << " retired " << hashes_matched_ << " matches "
          << (buffers_retired_ ? (total_bytes_ / buffers_retired_) : 0)
          << " bytes/buffer";
  return ostream.str();
}

FileOutputStream::~FileOutputStream() {
  while (!buffers_.empty()) {
    // Shake out any less-than-minimum-sized buffers that remain.
    EmitAndReleaseTopBuffer();
  }
  if (show_stats_) {
    fprintf(stderr, "%s\n", stats_.ToString().c_str());
    fflush(stderr);
  }
}

void FileOutputStream::EnqueueEntry(const proto::Entry &entry) {
  if (cache_ == &default_cache_ || buffers_.empty()) {
    {
      google::protobuf::io::CodedOutputStream coded_stream(stream_);
      coded_stream.WriteVarint32(entry.ByteSize());
      entry.SerializeToCodedStream(&coded_stream);
    }
    if (flush_after_each_entry_) {
      stream_->Flush();
    }
    return;
  }

  size_t entry_size = entry.ByteSize();
  size_t size_size =
      google::protobuf::io::CodedOutputStream::VarintSize32(entry_size);
  size_t size_delta = entry_size + size_size;
  unsigned char *buffer = buffers_.WriteToTop(size_delta);

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
