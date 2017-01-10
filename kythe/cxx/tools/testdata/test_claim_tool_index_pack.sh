#!/bin/bash -e
# This script checks that the claiming tool works on index packs.
BASE_DIR="$PWD/kythe/cxx/tools/testdata"
OUT_DIR="$TEST_TMPDIR"
KINDEX_TOOL_BIN="kythe/cxx/tools/kindex_tool"
CLAIM_TOOL_BIN="kythe/cxx/tools/static_claim"
INDEX_PACK_BIN="kythe/go/platform/tools/indexpack/indexpack"
mkdir -p "${OUT_DIR}"
rm -rf -- "${OUT_DIR}/pack"
"${KINDEX_TOOL_BIN}" -assemble "${OUT_DIR}/claim_test_1.kindex" \
  "${BASE_DIR}/claim_test_1.kindex_UNIT"
"${KINDEX_TOOL_BIN}" -assemble "${OUT_DIR}/claim_test_2.kindex" \
  "${BASE_DIR}/claim_test_2.kindex_UNIT"
"${INDEX_PACK_BIN}" --to_archive "${OUT_DIR}/pack" \
    "${OUT_DIR}"/claim_test_*.kindex >/dev/null
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
"${CLAIM_TOOL_BIN}" -text -index_pack "${OUT_DIR}/pack" \
    | diff "${BASE_DIR}/claim_test.expected" -
