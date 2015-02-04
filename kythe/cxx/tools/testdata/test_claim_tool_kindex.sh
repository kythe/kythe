#!/bin/bash -e
# This script checks that the claiming tool works on kindex files.
export CAMPFIRE_ROOT=`pwd`
export KINDEX_TOOL_BIN="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
export CLAIM_TOOL_BIN="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/tools/static_claim"
export TEST_DATA="${CAMPFIRE_ROOT}/kythe/cxx/tools/testdata"
export TEST_TEMP="${CAMPFIRE_ROOT}/campfire-out/test/kythe/cxx/tools/testdata/test_claim_tool_kindex"
mkdir -p "${TEST_TEMP}"
"${KINDEX_TOOL_BIN}" -assemble "${TEST_TEMP}/claim_test_1.kindex" \
  "${TEST_DATA}/claim_test_1.kindex_UNIT"
"${KINDEX_TOOL_BIN}" -assemble "${TEST_TEMP}/claim_test_2.kindex" \
  "${TEST_DATA}/claim_test_2.kindex_UNIT"
ls "${TEST_TEMP}"/claim_test_*.kindex | "${CLAIM_TOOL_BIN}" -text \
    | diff "${TEST_DATA}/claim_test.expected" -
