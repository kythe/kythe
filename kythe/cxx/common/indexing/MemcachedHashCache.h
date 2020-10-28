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

#ifndef KYTHE_CXX_COMMON_INDEXING_MEMCACHEDHASHCACHE_H_
#define KYTHE_CXX_COMMON_INDEXING_MEMCACHEDHASHCACHE_H_

#include <string>

#include "kythe/cxx/common/indexing/KytheCachingOutput.h"

extern "C" {
struct memcached_st;
}

namespace kythe {

/// \brief A `HashCache` that uses a memcached server.
class MemcachedHashCache : public HashCache {
 public:
  ~MemcachedHashCache() override;

  /// \brief Use a memcached instance (e.g. "--SERVER=foo:1234")
  bool OpenMemcache(const std::string& spec);

  void RegisterHash(const Hash& hash) override;

  bool SawHash(const Hash& hash) override;

 private:
  ::memcached_st* cache_ = nullptr;
};

}  // namespace kythe
#endif  // KYTHE_CXX_COMMON_INDEXING_MEMCACHEDHASHCACHE_H_
