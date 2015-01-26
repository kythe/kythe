#!/bin/bash -e
# This test checks that the extractor will emit index packs.
# It should be run from the Kythe root.
BASE_DIR="${PWD}/kythe/cxx/extractor/testdata"
OUT_DIR="${PWD}/campfire-out/test/kythe/cxx/extractor/testdata/index_pack"
EXTRACTOR="${PWD}/campfire-out/bin/kythe/cxx/extractor/cxx_extractor"
KINDEX_TOOL="${PWD}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
rm -rf -- "${OUT_DIR}"
mkdir -p "${OUT_DIR}"
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" KYTHE_INDEX_PACK="1" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/transcript_main.cc
# Storing redundant extractions is OK.
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" KYTHE_INDEX_PACK="1" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/transcript_main.cc
test -e "${OUT_DIR}/units" || exit 1
test -e "${OUT_DIR}/files" || exit 1
test -e "${OUT_DIR}/files/15d3490610af31dff6f1be9948ef61e66db48a84fc8fd93a81a5433abab04309.data" || exit 1
test -e "${OUT_DIR}/files/6904079b7b9d5d0586a08dbbdac2b08d35f00c1dbb4cc63721f22a347f52e2f7.data" || exit 1
test -e "${OUT_DIR}/files/7684704ae672c88bd4a656eff30a716ab29a8270e87e387d9101b32527cde498.data" || exit 1
test -e "${OUT_DIR}/units/daf513175d9a2222733647f6d3df0f22368e7440e4ca0b70669b78dbfb2672b0.unit" || exit 1
