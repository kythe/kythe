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
#ifndef DYNAMIC_CXX_INDEXER_CXX_DYNAMIC_CLAIM_CLIENT_H_
#define DYNAMIC_CXX_INDEXER_CXX_DYNAMIC_CLAIM_CLIENT_H_

#include <map>

#include "kythe/cxx/common/vname_ordering.h"
#include "kythe/cxx/indexer/cxx/KytheClaimClient.h"
#include "kythe/proto/storage.pb.h"

extern "C" {
struct memcached_st;
}

namespace kythe {

/// \brief A client that makes dynamic decisions about claiming by consulting
/// an external memcache.
///
/// This client is experimental and does not guarantee that entries will not be
/// permanently dropped.
class DynamicClaimClient : public KytheClaimClient {
 public:
  ~DynamicClaimClient() override;

  /// \brief Use a memcached instance (e.g. "--SERVER=foo:1234")
  bool OpenMemcache(const std::string& spec);

  bool Claim(const kythe::proto::VName& claimant,
             const kythe::proto::VName& vname) override;

  /// Store a local override.
  void AssignClaim(const kythe::proto::VName& claimable,
                   const kythe::proto::VName& claimant) override;

  /// Change how many times the same VName can be claimed.
  void set_max_redundant_claims(size_t value) { max_redundant_claims_ = value; }

  void Reset() override { claim_table_.clear(); }

 private:
  /// A local map from claimables to claimants.
  std::map<kythe::proto::VName, kythe::proto::VName, VNameLess> claim_table_;
  /// A remote map used for dynamic queries.
  ::memcached_st* cache_ = nullptr;
  /// The maximum number of times a VName can be claimed.
  size_t max_redundant_claims_ = 1;
  /// The number of claim requests ever made.
  size_t request_count_ = 0;
  /// The number of claim requests that were rejected (after all tries).
  size_t rejected_requests_ = 0;
};

}  // namespace kythe

#endif  // DYNAMIC_CXX_INDEXER_CXX_DYNAMIC_CLAIM_CLIENT_H_
