#!/bin/bash -e
# This script checks that the claiming tool works on kindex files.
BASE_DIR="$PWD/kythe/cxx/tools/testdata"
OUT_DIR="$TEST_TMPDIR"
KINDEX_TOOL_BIN="kythe/cxx/tools/kindex_tool"
CLAIM_TOOL_BIN="kythe/cxx/tools/static_claim"

mkdir -p "${OUT_DIR}"
"${KINDEX_TOOL_BIN}" -assemble "${OUT_DIR}/claim_test_1.kindex" \
  "${BASE_DIR}/claim_test_1.kindex_UNIT"
"${KINDEX_TOOL_BIN}" -assemble "${OUT_DIR}/claim_test_2.kindex" \
  "${BASE_DIR}/claim_test_2.kindex_UNIT"
ls "${OUT_DIR}"/claim_test_*.kindex | "${CLAIM_TOOL_BIN}" -text \
    | diff "${BASE_DIR}/claim_test.expected" -
