#!/bin/bash -e
# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# This test checks that transcripts do not record useless changes in the
# environment. Specifically, the main source file transcript for the
# _without.UNIT file should equal the main source file transcript for the
# _with.UNIT file.
# It should be run from the Kythe root.
TEST_NAME="test_main_source_file_no_env_dep"
. ./kythe/cxx/extractor/testdata/test_common.sh
. ./kythe/cxx/extractor/testdata/skip_functions.sh
mkdir -p "${OUT_DIR}/with"
mkdir -p "${OUT_DIR}/without"
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}/without" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}/with" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    -DMACRO ./kythe/cxx/extractor/testdata/main_source_file_no_env_dep.cc
[[ $(ls -1 "${OUT_DIR}"/with/*.kindex | wc -l) -eq 1 ]]
INDEX_PATH_WITH_MACRO=$(ls -1 "${OUT_DIR}"/with/*.kindex)
[[ $(ls -1 "${OUT_DIR}"/without/*.kindex | wc -l) -eq 1 ]]
INDEX_PATH_WITHOUT_MACRO=$(ls -1 "${OUT_DIR}"/without/*.kindex)
"${KINDEX_TOOL}" -suppress_details -explode "${INDEX_PATH_WITH_MACRO}"
"${KINDEX_TOOL}" -suppress_details -explode "${INDEX_PATH_WITHOUT_MACRO}"

# Remove lines that will change depending on the machine the test is run on.
skip_inplace "-target" 1 "${INDEX_PATH_WITH_MACRO}_UNIT"
skip_inplace "signature" 0 "${INDEX_PATH_WITH_MACRO}_UNIT"
skip_inplace "-target" 1 "${INDEX_PATH_WITHOUT_MACRO}_UNIT"
skip_inplace "signature" 0 "${INDEX_PATH_WITHOUT_MACRO}_UNIT"

EC_HASH=$(sed -ne '/^entry_context:/ {s/.*entry_context: \"\(.*\)\"$/\1/; p;}' \
    "${INDEX_PATH_WITH_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g
s|TEST_CWD|${PWD}/|" \
    "${BASE_DIR}/main_source_file_no_env_dep_with.UNIT" | \
    skip "-target" 1 |
    skip "signature" 0 |
    diff - "${INDEX_PATH_WITH_MACRO}_UNIT"

EC_HASH=$(sed -ne '/^entry_context:/ {s/.*entry_context: \"\(.*\)\"$/\1/; p;}' \
    "${INDEX_PATH_WITHOUT_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g
s|TEST_CWD|${PWD}/|" \
    "${BASE_DIR}/main_source_file_no_env_dep_without.UNIT" | \
    skip "-target" 1 |
    skip "signature" 0 |
    diff - "${INDEX_PATH_WITHOUT_MACRO}_UNIT"
