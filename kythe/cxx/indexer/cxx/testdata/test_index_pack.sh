#!/bin/bash -e
# Tests whether the indexer will read from kindex files.
CAMPFIRE_ROOT=${PWD}
BASEDIR="${CAMPFIRE_ROOT}/kythe/cxx/indexer/cxx/testdata"
VERIFIER="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/verifier/verifier"
INDEXER="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/indexer/cxx/indexer"
KINDEX_TOOL="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
INDEX_PACK_BIN="${CAMPFIRE_ROOT}/campfire-out/bin/kythe/go/platform/tools/indexpack"
TEST_TEMP="${CAMPFIRE_ROOT}/campfire-out/test/kythe/cxx/indexer/cxx/testdata/test_index_pack"
TEST_INDEX="${TEST_TEMP}/test.kindex"
mkdir -p "${TEST_TEMP}"
rm -rf -- "${TEST_TEMP}/pack"
"${KINDEX_TOOL}" -assemble "${TEST_INDEX}" \
    "${BASEDIR}/kindex_test.unit" \
    "${BASEDIR}/kindex_test.header" \
    "${BASEDIR}/kindex_test.main"
"${INDEX_PACK_BIN}" -quiet=true --to_archive "${TEST_TEMP}/pack" \
    "${TEST_INDEX}"
"${INDEXER}" -index_pack "${TEST_TEMP}/pack" \
    "f0f3ce43c41682991103e844cfc9ed7315624ffab3784bf701cd197236805c1b" \
    > "${TEST_TEMP}/kindex_test.entries"
cat "${TEST_TEMP}/kindex_test.entries" \
    | "${VERIFIER}" "${BASEDIR}/kindex_test.verify"
