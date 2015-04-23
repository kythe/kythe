#!/bin/bash -e
# Tests whether the indexer will read from kindex files.
KYTHE_BIN="${TEST_SRCDIR:-${PWD}/campfire-out/bin}"
BASE_DIR="${TEST_SRCDIR:-${PWD}}/kythe/cxx/indexer/cxx/testdata"
OUT_DIR="${TEST_TMPDIR:-${PWD}/campfire-out/test/kythe/cxx/indexer/cxx/testdata}"
VERIFIER="${KYTHE_BIN}/kythe/cxx/verifier/verifier"
INDEXER="${KYTHE_BIN}/kythe/cxx/indexer/cxx/indexer"
KINDEX_TOOL="${KYTHE_BIN}/kythe/cxx/tools/kindex_tool"
TEST_INDEX="${OUT_DIR}/test.kindex"
REPO_TEST_INDEX="${OUT_DIR}/repo_test.kindex"
mkdir -p "${OUT_DIR}"
"${KINDEX_TOOL}" -assemble "${TEST_INDEX}" \
    "${BASE_DIR}/kindex_test.unit" \
    "${BASE_DIR}/kindex_test.header" \
    "${BASE_DIR}/kindex_test.main"
"${INDEXER}" "${TEST_INDEX}" > "${OUT_DIR}/kindex_test.entries"
cat "${OUT_DIR}/kindex_test.entries" \
    | "${VERIFIER}" "${BASE_DIR}/kindex_test.verify"
# The second test (which is useless unless the first succeeds) checks that
# we handle relative paths.
"${KINDEX_TOOL}" -assemble "${REPO_TEST_INDEX}" \
    "${BASE_DIR}/kindex_repo_test.unit" \
    "${BASE_DIR}/kindex_repo_test.header" \
    "${BASE_DIR}/kindex_repo_test.main"
"${INDEXER}" "${REPO_TEST_INDEX}" > "${OUT_DIR}/kindex_repo_test.entries"
cat "${OUT_DIR}/kindex_repo_test.entries" \
    | "${VERIFIER}" "${BASE_DIR}/kindex_repo_test.verify"
