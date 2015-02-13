#!/bin/bash -e
# This test checks that the extractor handles #pragma kythe_claim.
# It should be run from the Kythe root.
BASE_DIR="${PWD}/kythe/cxx/extractor/testdata"
OUT_DIR="${PWD}/campfire-out/test/kythe/cxx/extractor/testdata"
EXTRACTOR="${PWD}/campfire-out/bin/kythe/cxx/extractor/cxx_extractor"
KINDEX_TOOL="${PWD}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
EXPECTED_INDEX="701540b440f4a705d068bf22dca4963251447feb72b2cdecd24c83ac369dedf7.kindex"
INDEX_PATH="${OUT_DIR}"/"${EXPECTED_INDEX}"
rm -f -- "${INDEX_PATH}_UNIT"
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/claim_main.cc
"${KINDEX_TOOL}" -suppress_header_info -explode "${INDEX_PATH}"
diff "${BASE_DIR}/claim_main.UNIT" "${INDEX_PATH}_UNIT"
