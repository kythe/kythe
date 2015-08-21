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
INDEX_WITH_MACRO="afac81524d5df80e01ae7ee5c135736179520692b816883948e35c5a8e55d76f.kindex"
INDEX_WITHOUT_MACRO="9337bc05db5374eb15dfd889d392707dec51b121490ce246f70495b1f7bb1c5c.kindex"
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
EC_HASH=$(sed -ne '/^entry_context:/ {s/.*entry_context: \"\(.*\)\"$/\1/; p;}' \
    "${INDEX_PATH_WITH_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g" \
    "${BASE_DIR}/main_source_file_no_env_dep_with.UNIT" \
    | diff - "${INDEX_PATH_WITH_MACRO}_UNIT"
EC_HASH=$(sed -ne '/^entry_context:/ {s/.*entry_context: \"\(.*\)\"$/\1/; p;}' \
    "${INDEX_PATH_WITHOUT_MACRO}_UNIT")
sed "s/EC_HASH/${EC_HASH}/g" \
    "${BASE_DIR}/main_source_file_no_env_dep_without.UNIT" \
    | diff - "${INDEX_PATH_WITHOUT_MACRO}_UNIT"
