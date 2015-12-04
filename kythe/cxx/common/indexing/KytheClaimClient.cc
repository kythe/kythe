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

#include "KytheClaimClient.h"

#include <libmemcached/memcached.h>
#include <openssl/sha.h>

namespace kythe {

bool StaticClaimClient::Claim(const kythe::proto::VName &claimant,
                              const kythe::proto::VName &vname) {
  const auto lookup = claim_table_.find(vname);
  if (lookup == claim_table_.end()) {
    // We don't know who's responsible for this VName.
    return process_unknown_status_;
  }
  return VNameEquals(lookup->second, claimant);
}

void StaticClaimClient::AssignClaim(const kythe::proto::VName &claimable,
                                    const kythe::proto::VName &claimant) {
  claim_table_[claimable] = claimant;
}

DynamicClaimClient::~DynamicClaimClient() {
  if (cache_) {
    memcached_free(cache_);
    cache_ = nullptr;
  }
  fprintf(
      stderr, "%8lu  %8lu claims approved/rejected (%f reject fraction)\n",
      request_count_ - rejected_requests_, rejected_requests_,
      request_count_ == 0 ? 0.0 : (double)rejected_requests_ / request_count_);
}

bool DynamicClaimClient::OpenMemcache(const std::string &spec) {
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

using Hash = unsigned char[SHA256_DIGEST_LENGTH];

void HashVName(const kythe::proto::VName &vname, size_t count, Hash *hash) {
  ::SHA256_CTX sha;
  ::SHA256_Init(&sha);
  ::SHA256_Update(&sha, vname.signature().c_str(), vname.signature().size());
  ::SHA256_Update(&sha, vname.path().c_str(), vname.path().size());
  ::SHA256_Update(&sha, vname.language().c_str(), vname.language().size());
  ::SHA256_Update(&sha, vname.root().c_str(), vname.root().size());
  ::SHA256_Update(&sha, vname.corpus().c_str(), vname.corpus().size());
  ::SHA256_Update(&sha, &count, sizeof(count));
  ::SHA256_Final(reinterpret_cast<unsigned char *>(hash), &sha);
}

bool DynamicClaimClient::Claim(const kythe::proto::VName &claimant,
                               const kythe::proto::VName &vname) {
  // TODO(zarko): (It may not matter because this is already
  // lossy, but) we only check to see whether anyone has made a claim on a
  // vname at any point; if not, then we assume that we own that vname.
  // If a claim exists in the cache (and isn't in the local claim_table_),
  // regardless of whether it was ours, we assume that we do not own it.
  ++request_count_;
  const auto lookup = claim_table_.find(vname);
  if (lookup == claim_table_.end()) {
    if (!cache_) {
      // Fail open.
      return true;
    }
    Hash claimant_hash, vname_hash;
    HashVName(claimant, 0, &claimant_hash);
    for (size_t tries = 0; tries < max_redundant_claims_; ++tries) {
      HashVName(vname, tries, &vname_hash);
      memcached_return_t add_result = memcached_add(
          cache_, reinterpret_cast<const char *>(&vname_hash),
          SHA256_DIGEST_LENGTH, reinterpret_cast<const char *>(&claimant_hash),
          SHA256_DIGEST_LENGTH, 0, 0);
      if (!memcached_success(add_result) &&
          add_result != MEMCACHED_DATA_EXISTS) {
        // We'll also pass the check below and assume we claimed the vname.
        fprintf(stderr, "memcached add failed: %s\n",
                memcached_strerror(cache_, add_result));
      }
      if (add_result != MEMCACHED_DATA_EXISTS) {
        claim_table_[vname] = claimant;
        return true;
      }
    }
    // We failed all our tries, so assume we couldn't make a claim.
    claim_table_[vname] = kythe::proto::VName();
    ++rejected_requests_;
    return false;
  }

  if (VNameEquals(lookup->second, claimant)) {
    return true;
  } else {
    ++rejected_requests_;
    return false;
  }
}

void DynamicClaimClient::AssignClaim(const kythe::proto::VName &claimable,
                                     const kythe::proto::VName &claimant) {
  claim_table_[claimable] = claimant;
}
}
