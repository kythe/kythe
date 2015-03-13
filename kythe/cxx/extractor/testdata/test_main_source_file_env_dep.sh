#!/bin/bash -e
# This test checks that transcripts record useful changes in the environment.
# Specifically, the main source file transcript for the _without.UNIT file
# should differ from the main source file transcript for the _with.UNIT file.
# It should be run from the Kythe root.
BASE_DIR="${PWD}/kythe/cxx/extractor/testdata"
OUT_DIR="${PWD}/campfire-out/test/kythe/cxx/extractor/testdata"
EXTRACTOR="${PWD}/campfire-out/bin/kythe/cxx/extractor/cxx_extractor"
KINDEX_TOOL="${PWD}/campfire-out/bin/kythe/cxx/tools/kindex_tool"
INDEX_WITH_MACRO="092175fd1d6172588ef89d4d6a0979460fef9eaa5b28b32fde5f38895962aab0.kindex"
INDEX_WITHOUT_MACRO="dffcbf93cf2aa88c7a32245e36e0645c481edcfbfb6484bb6ee0d2b0363439a7.kindex"
INDEX_PATH_WITH_MACRO="${OUT_DIR}"/"${INDEX_WITH_MACRO}"
INDEX_PATH_WITHOUT_MACRO="${OUT_DIR}"/"${INDEX_WITHOUT_MACRO}"
rm -f -- "${INDEX_PATH_WITH_MACRO}_UNIT" "${INDEX_PATH_WITHOUT_MACRO}_UNIT"
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/main_source_file_env_dep.cc
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata -DMACRO \
    ./kythe/cxx/extractor/testdata/main_source_file_env_dep.cc
"${KINDEX_TOOL}" -suppress_details -explode "${INDEX_PATH_WITH_MACRO}"
"${KINDEX_TOOL}" -suppress_details -explode "${INDEX_PATH_WITHOUT_MACRO}"
EC_HASH=$(grep -oP 'entry_context: \"\K.*\"$' \
    "${INDEX_PATH_WITH_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g" \
    "${BASE_DIR}/main_source_file_env_dep_with.UNIT" \
    | diff - "${INDEX_PATH_WITH_MACRO}_UNIT"
EC_HASH=$(grep -oP 'entry_context: \"\K.*\"$' \
    "${INDEX_PATH_WITHOUT_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g" \
    "${BASE_DIR}/main_source_file_env_dep_without.UNIT" \
    | diff - "${INDEX_PATH_WITHOUT_MACRO}_UNIT"
