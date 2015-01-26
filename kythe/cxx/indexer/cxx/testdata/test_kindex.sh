#!/bin/bash -e
# Tests whether the indexer will read from kindex files.
CAMPFIRE_ROOT=${PWD}
BASEDIR="${CAMPFIRE_ROOT}/kythe/cxx/indexer/cxx/testdata"
VERIFIER="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/verifier/verifier"
INDEXER="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/indexer/cxx/indexer"
KINDEX_TOOL="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
TEST_TEMP="${CAMPFIRE_ROOT}/campfire-out/test/kythe/cxx/indexer/cxx/testdata"
TEST_INDEX="${TEST_TEMP}/test.kindex"
REPO_TEST_INDEX="${TEST_TEMP}/repo_test.kindex"
mkdir -p "${TEST_TEMP}"
"${KINDEX_TOOL}" -assemble "${TEST_INDEX}" \
    "${BASEDIR}/kindex_test.unit" \
    "${BASEDIR}/kindex_test.header" \
    "${BASEDIR}/kindex_test.main"
"${INDEXER}" "${TEST_INDEX}" > "${TEST_TEMP}/kindex_test.entries"
cat "${TEST_TEMP}/kindex_test.entries" \
    | "${VERIFIER}" "${BASEDIR}/kindex_test.verify"
# The second test (which is useless unless the first succeeds) checks that
# we handle relative paths.
"${KINDEX_TOOL}" -assemble "${REPO_TEST_INDEX}" \
    "${BASEDIR}/kindex_repo_test.unit" \
    "${BASEDIR}/kindex_repo_test.header" \
    "${BASEDIR}/kindex_repo_test.main"
"${INDEXER}" "${REPO_TEST_INDEX}" > "${TEST_TEMP}/kindex_repo_test.entries"
cat "${TEST_TEMP}/kindex_repo_test.entries" \
    | "${VERIFIER}" "${BASEDIR}/kindex_repo_test.verify"
