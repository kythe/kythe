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
# This test checks that the extractor will emit index packs.
# It should be run from the Kythe root.
TEST_NAME="test_index_pack"
. ./kythe/cxx/extractor/testdata/test_common.sh
. ./kythe/cxx/extractor/testdata/skip_functions.sh
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
[[ $(ls -1 "${OUT_DIR}"/files/*.data | wc -l) -eq 3 ]]
[[ $(ls -1 "${OUT_DIR}"/units/*.unit | wc -l) -eq 1 ]]
