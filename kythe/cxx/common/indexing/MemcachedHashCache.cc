/*
 * Copyright 2019 The Kythe Authors. All rights reserved.
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

#include "kythe/cxx/common/indexing/MemcachedHashCache.h"

#include <libmemcached-1.0/memcached.h>

#include <iostream>
#include <string>

namespace kythe {

bool MemcachedHashCache::OpenMemcache(const std::string& spec) {
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

void MemcachedHashCache::RegisterHash(const Hash& hash) {
  if (!cache_) {
    return;
  }
  char value = 1;
  memcached_return_t add_result =
      memcached_add(cache_, reinterpret_cast<const char*>(hash), kHashSize,
                    &value, sizeof(value), 0, 0);
  if (!memcached_success(add_result) && add_result != MEMCACHED_DATA_EXISTS) {
    std::cerr << "memcached add failed: "
              << memcached_strerror(cache_, add_result) << "\n";
  }
}

bool MemcachedHashCache::SawHash(const Hash& hash) {
  if (!cache_) {
    return false;
  }
  memcached_return_t ex_result =
      memcached_exist(cache_, reinterpret_cast<const char*>(hash), kHashSize);
  if (ex_result == MEMCACHED_SUCCESS) {
    return true;
  } else if (ex_result != MEMCACHED_NOTFOUND) {
    std::cerr << "memcached exist failed: "
              << memcached_strerror(cache_, ex_result) << "\n";
  }
  return false;
}

}  // namespace kythe
