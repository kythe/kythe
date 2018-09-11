#!/bin/bash -e
# Tests whether the indexer will read from kindex files.
BASE_DIR="$PWD/kythe/cxx/indexer/cxx/testdata"
OUT_DIR="$TEST_TMPDIR"
: ${VERIFIER?:missing verifier}
: ${INDEXER?:missing indexer}
: ${INDEX_PACK_BIN?:missing indexpack}
: ${KINDEX_TOOL?:missing kindex_tool}
TEST_INDEX="${OUT_DIR}/test.kindex"
REPO_TEST_INDEX="${OUT_DIR}/repo_test.kindex"
mkdir -p "${OUT_DIR}"
rm -rf -- "${OUT_DIR}/pack"
"${KINDEX_TOOL}" -assemble "${TEST_INDEX}" \
    "${BASE_DIR}/kindex_test.unit" \
    "${BASE_DIR}/kindex_test.header" \
    "${BASE_DIR}/kindex_test.main"
"${INDEX_PACK_BIN}" --to_archive "${OUT_DIR}/pack" \
    "${TEST_INDEX}"
"${INDEXER}" --ignore_unimplemented=false -index_pack "${OUT_DIR}/pack" \
    "401bdc75a298d6c3a11a10f493b182032793034fddd84e9810f89b5def902309" \
    > "${OUT_DIR}/kindex_test.entries"
cat "${OUT_DIR}/kindex_test.entries" \
    | "${VERIFIER}" --nocheck_for_singletons "${BASE_DIR}/kindex_test.verify"
