#include "KytheClaimClient.h"

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
}
