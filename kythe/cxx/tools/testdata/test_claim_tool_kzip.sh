#!/bin/bash
# This script checks that the claiming tool works on kindex files.
set -e
BASE_DIR="$PWD/kythe/cxx/tools/testdata"
OUT_DIR="$TEST_TMPDIR"
: ${CLAIM_TOOL_BIN?:missing static_claim}

mkdir -p "${OUT_DIR}/tmp/units" "${OUT_DIR}/tmp/files"
cp "${BASE_DIR}"/claim_test_{1,2}.kzip_UNIT.json "${OUT_DIR}/tmp/units"
(cd "${OUT_DIR}"; zip -r claim_test.kzip tmp)
ls "${OUT_DIR}"/claim_test.kzip | "${CLAIM_TOOL_BIN}" -text \
    | diff "${BASE_DIR}/claim_test.expected" -
