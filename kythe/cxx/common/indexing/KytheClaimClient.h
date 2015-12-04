/*
 * Copyright 2015 Google Inc. All rights reserved.
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

#ifndef KYTHE_CXX_COMMON_INDEXING_KYTHE_CLAIM_CLIENT_H_
#define KYTHE_CXX_COMMON_INDEXING_KYTHE_CLAIM_CLIENT_H_

#include <map>

#include "kythe/cxx/common/vname_ordering.h"
#include "kythe/proto/storage.pb.h"

extern "C" {
struct memcached_st;
}

namespace kythe {

/// \brief Limits redundancy in indexer output by skipping over certain
/// entities.
class KytheClaimClient {
 public:
  virtual ~KytheClaimClient() {}
  /// \brief Makes a claim for the provided `vname`.
  /// \param claimant A VName that identifies the claimant.
  /// \param vname A VName for the resource being claimed.
  /// \return true if `claimant` is responsible for `vname`.
  virtual bool Claim(const kythe::proto::VName &claimant,
                     const kythe::proto::VName &vname) {
    return true;
  }
  /// \brief Assigns responsibility for `claimable` to `claimant`.
  virtual void AssignClaim(const kythe::proto::VName &claimable,
                           const kythe::proto::VName &claimant) = 0;

  /// \brief Resets any cached state, including any claims made by
  /// `AssignClaim`.
  virtual void Reset() {}
};

/// \brief A client that makes static decisions about resources when possible.
class StaticClaimClient : public KytheClaimClient {
 public:
  bool Claim(const kythe::proto::VName &claimant,
             const kythe::proto::VName &vname) override;

  /// \brief Process data with unknown claim status?
  ///
  /// If true, then entities without claimants assigned will be
  /// processed by every claimant.
  void set_process_unknown_status(bool process_unknown_status) {
    process_unknown_status_ = process_unknown_status;
  }

  void AssignClaim(const kythe::proto::VName &claimable,
                   const kythe::proto::VName &claimant) override;

  void Reset() override { claim_table_.clear(); }

 private:
  /// Maps from claimables to claimants.
  std::map<kythe::proto::VName, kythe::proto::VName, VNameLess> claim_table_;
  /// Process data with unknown claim status?
  bool process_unknown_status_ = true;
};

/// \brief A client that makes dynamic decisions about claiming by consulting
/// an external memcache.
///
/// This client is experimental and does not guarantee that entries will not be
/// permanently dropped.
class DynamicClaimClient : public KytheClaimClient {
 public:
  ~DynamicClaimClient() override;

  /// \brief Use a memcached instance (e.g. "--SERVER=foo:1234")
  bool OpenMemcache(const std::string &spec);

  bool Claim(const kythe::proto::VName &claimant,
             const kythe::proto::VName &vname) override;

  /// Store a local override.
  void AssignClaim(const kythe::proto::VName &claimable,
                   const kythe::proto::VName &claimant) override;

  /// Change how many times the same VName can be claimed.
  void set_max_redundant_claims(size_t value) { max_redundant_claims_ = value; }

  void Reset() override { claim_table_.clear(); }

 private:
  /// A local map from claimables to claimants.
  std::map<kythe::proto::VName, kythe::proto::VName, VNameLess> claim_table_;
  /// A remote map used for dynamic queries.
  ::memcached_st *cache_ = nullptr;
  /// The maximum number of times a VName can be claimed.
  size_t max_redundant_claims_ = 1;
  /// The number of claim requests ever made.
  size_t request_count_ = 0;
  /// The number of claim requests that were rejected (after all tries).
  size_t rejected_requests_ = 0;
};

}  // namespace kythe

#endif
