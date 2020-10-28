/*
 * Copyright 2015 The Kythe Authors. All rights reserved.
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

namespace kythe {
namespace {
constexpr char kArbitraryClaimantRoot[] = "KytheClaimClient";
}  // anonymous namespace

bool KytheClaimClient::ClaimBatch(
    std::vector<std::pair<std::string, bool>>* tokens) {
  kythe::proto::VName claim;
  claim.set_root(kArbitraryClaimantRoot);
  bool success = false;
  for (auto& token : *tokens) {
    claim.set_signature(token.first);
    if ((token.second = Claim(claim, claim))) {
      success = true;
    }
  }
  return success;
}

bool StaticClaimClient::Claim(const kythe::proto::VName& claimant,
                              const kythe::proto::VName& vname) {
  const auto lookup = claim_table_.find(vname);
  if (lookup == claim_table_.end()) {
    // We don't know who's responsible for this VName.
    return process_unknown_status_;
  }
  return VNameEquals(lookup->second, claimant);
}

void StaticClaimClient::AssignClaim(const kythe::proto::VName& claimable,
                                    const kythe::proto::VName& claimant) {
  claim_table_[claimable] = claimant;
}

}  // namespace kythe
