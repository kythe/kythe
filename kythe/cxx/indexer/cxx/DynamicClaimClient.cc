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
#include "kythe/cxx/indexer/cxx/DynamicClaimClient.h"

#include <libmemcached-1.0/memcached.h>

#include <array>
#include <cstddef>

#include "absl/strings/str_format.h"
#include "kythe/cxx/common/sha256_hasher.h"
#include "kythe/cxx/common/vname_ordering.h"

namespace kythe {
namespace {
using Hash = std::array<std::byte, SHA256_DIGEST_LENGTH>;

Hash HashVName(const kythe::proto::VName& vname, size_t count) {
  Sha256Hasher hash;
  hash.Update(vname.signature());
  hash.Update(vname.path());
  hash.Update(vname.language());
  hash.Update(vname.root());
  hash.Update(vname.corpus());
  hash.Update({reinterpret_cast<const char*>(&count), sizeof(count)});
  return std::move(hash).Finish();
}
}  // namespace

DynamicClaimClient::~DynamicClaimClient() {
  if (cache_) {
    memcached_free(cache_);
    cache_ = nullptr;
  }
  absl::FPrintF(stderr,
                "%8lu  %8lu claims approved/rejected (%f reject fraction)\n",
                request_count_ - rejected_requests_, rejected_requests_,
                request_count_ == 0
                    ? 0.0
                    : static_cast<double>(rejected_requests_) / request_count_);
}

bool DynamicClaimClient::OpenMemcache(const std::string& spec) {
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

bool DynamicClaimClient::Claim(const kythe::proto::VName& claimant,
                               const kythe::proto::VName& vname) {
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
    Hash claimant_hash = HashVName(claimant, 0);
    for (size_t tries = 0; tries < max_redundant_claims_; ++tries) {
      Hash vname_hash = HashVName(vname, tries);
      memcached_return_t add_result = memcached_add(
          cache_, reinterpret_cast<const char*>(vname_hash.data()),
          vname_hash.size(),
          reinterpret_cast<const char*>(claimant_hash.data()),
          claimant_hash.size(), 0, 0);
      if (!memcached_success(add_result) &&
          add_result != MEMCACHED_DATA_EXISTS) {
        // We'll also pass the check below and assume we claimed the vname.
        absl::FPrintF(stderr, "memcached add failed: %s\n",
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

void DynamicClaimClient::AssignClaim(const kythe::proto::VName& claimable,
                                     const kythe::proto::VName& claimant) {
  claim_table_[claimable] = claimant;
}
}  // namespace kythe
