#!/bin/bash -e
# This test checks that the extractor handles transcripts.
# It should be run from the Kythe root.
BASE_DIR="${PWD}/kythe/cxx/extractor/testdata"
OUT_DIR="${PWD}/campfire-out/test/kythe/cxx/extractor/testdata"
EXTRACTOR="${PWD}/campfire-out/bin/kythe/cxx/extractor/cxx_extractor"
KINDEX_TOOL="${PWD}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
EXPECTED_INDEX="4a9aa76e4ce8a7d496e9c24d1ef114887292b86e014c60c4b774c2c19224bdcf.kindex"
INDEX_PATH="${OUT_DIR}"/"${EXPECTED_INDEX}"
rm -f -- "${INDEX_PATH}_UNIT"
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/transcript_main.cc
"${KINDEX_TOOL}" -explode "${INDEX_PATH}"
diff "${BASE_DIR}/transcript_main.UNIT" "${INDEX_PATH}_UNIT"
