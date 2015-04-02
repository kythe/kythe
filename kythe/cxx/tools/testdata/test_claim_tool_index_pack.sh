#!/bin/bash -e
# This script checks that the claiming tool works on index packs.
export CAMPFIRE_ROOT=`pwd`
export KINDEX_TOOL_BIN="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
export CLAIM_TOOL_BIN="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/tools/static_claim"
export INDEX_PACK_BIN="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/go/platform/tools/indexpack"
export TEST_DATA="${CAMPFIRE_ROOT}/kythe/cxx/tools/testdata"
export TEST_TEMP="${CAMPFIRE_ROOT}/campfire-out/test/kythe/cxx/tools/testdata/test_claim_tool_index_pack"
mkdir -p "${TEST_TEMP}"
rm -rf -- "${TEST_TEMP}/pack"
"${KINDEX_TOOL_BIN}" -assemble "${TEST_TEMP}/claim_test_1.kindex" \
  "${TEST_DATA}/claim_test_1.kindex_UNIT"
"${KINDEX_TOOL_BIN}" -assemble "${TEST_TEMP}/claim_test_2.kindex" \
  "${TEST_DATA}/claim_test_2.kindex_UNIT"
"${INDEX_PACK_BIN}" -quiet=true --to_archive "${TEST_TEMP}/pack" \
    "${TEST_TEMP}"/claim_test_*.kindex >/dev/null
# The assignment is arbitrary but should be stable, since the heuristic for
# assigning responsibility for a claimable picks the first possible claimant
# with the lowest claim count (and this might differ for the same index pack
# depending on how the claimant set is ordered).
# In one ordering (claim_test_1, claim_test_2) and (b.h, a.h_1, a.h_2, c.h_1):
#   b.h = claim_test_2 (only possible)
#   a.h_1 = claim_test_1 (#2 has 1 claim)
#   a.h_2 = claim_test_1 (both have 1 claim but _1 is arbitrarily earlier)
#   c.h = claim_test_1 (only possible)
# If the ordering were flipped to (claim_test_2, claim_test_1), a.h_2 would
# be assigned to claim_test_2.
"${CLAIM_TOOL_BIN}" -text -index_pack "${TEST_TEMP}/pack" \
    | diff "${TEST_DATA}/claim_test.expected" -
