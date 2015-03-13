#!/bin/bash -e
# This test checks that transcripts do not record useless changes in the
# environment. Specifically, the main source file transcript for the
# _without.UNIT file should equal the main source file transcript for the
# _with.UNIT file.
# It should be run from the Kythe root.
BASE_DIR="${PWD}/kythe/cxx/extractor/testdata"
OUT_DIR="${PWD}/campfire-out/test/kythe/cxx/extractor/testdata"
EXTRACTOR="${PWD}/campfire-out/bin/kythe/cxx/extractor/cxx_extractor"
KINDEX_TOOL="${PWD}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
INDEX_WITH_MACRO="290c611e6db23cda70a449b702c4a21f5866a7483d0d39afb4a059c283080886.kindex"
INDEX_WITHOUT_MACRO="aff6a6ee4f740ec4334c80afacfef81fef94ffe34280f40552deb4c72e465653.kindex"
INDEX_PATH_WITH_MACRO="${OUT_DIR}"/"${INDEX_WITH_MACRO}"
INDEX_PATH_WITHOUT_MACRO="${OUT_DIR}"/"${INDEX_WITHOUT_MACRO}"
rm -f -- "${INDEX_PATH_WITH_MACRO}_UNIT" "${INDEX_PATH_WITHOUT_MACRO}_UNIT"
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    -DMACRO ./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc
"${KINDEX_TOOL}" -suppress_details -explode "${INDEX_PATH_WITH_MACRO}"
"${KINDEX_TOOL}" -suppress_details -explode "${INDEX_PATH_WITHOUT_MACRO}"
EC_HASH=$(grep -oP 'entry_context: \"\K.*\"$' \
    "${INDEX_PATH_WITH_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g" \
    "${BASE_DIR}/main_source_file_no_env_dep_with.UNIT" \
    | diff - "${INDEX_PATH_WITH_MACRO}_UNIT"
EC_HASH=$(grep -oP 'entry_context: \"\K.*\"$' \
    "${INDEX_PATH_WITHOUT_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g" \
    "${BASE_DIR}/main_source_file_no_env_dep_without.UNIT" \
    | diff - "${INDEX_PATH_WITHOUT_MACRO}_UNIT"
