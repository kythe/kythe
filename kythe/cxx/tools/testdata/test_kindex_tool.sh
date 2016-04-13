#!/bin/bash
# This script checks that kindex_tool -assemble is inverted by
# kindex_tool -explode.
BASE_DIR="$PWD/kythe/cxx/tools/testdata"
OUT_DIR="$TEST_TMPDIR"
KINDEX_TOOL_BIN="kythe/cxx/tools/kindex_tool"

set -e
mkdir -p "${OUT_DIR}"
"${KINDEX_TOOL_BIN}" -assemble "${OUT_DIR}/test.kindex" \
  "${BASE_DIR}/java.kindex_UNIT" \
  "${BASE_DIR}/java.kindex_cf28b786fa21d0c45156e8011ac809afc454703fa03d767a5aeeed382f902795"
"${KINDEX_TOOL_BIN}" -explode "${OUT_DIR}/test.kindex"
diff "${BASE_DIR}/java.kindex_UNIT" "${OUT_DIR}/test.kindex_UNIT"
diff "${BASE_DIR}/java.kindex_cf28b786fa21d0c45156e8011ac809afc454703fa03d767a5aeeed382f902795" \
    "${OUT_DIR}/test.kindex_cf28b786fa21d0c45156e8011ac809afc454703fa03d767a5aeeed382f902795"
