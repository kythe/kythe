#!/bin/bash
# Copyright 2015 The Kythe Authors. All rights reserved.
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
# This test checks that the extractor will emit kzip files.
# It should be run from the Kythe root.
set -e
TEST_NAME="test_index_pack"
. ./kythe/cxx/extractor/testdata/test_common.sh
. ./kythe/cxx/extractor/testdata/skip_functions.sh
rm -rf -- "${OUT_DIR}"
mkdir -p "${OUT_DIR}"
KYTHE_OUTPUT_FILE="${OUT_DIR}/compilations.kzip" \
    "./${EXTRACTOR}" --with_executable "/dummy/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/transcript_main.cc
# Storing redundant extractions should fail if the file exists.
# TODO(shahms): Test this.
#KYTHE_OUTPUT_FILE="${OUT_DIR}/compilations.kzip" \
#    "./${EXTRACTOR}" --with_executable "/dummy/bin/g++" \
#    -I./kythe/cxx/extractor/testdata \
#    ./kythe/cxx/extractor/testdata/transcript_main.cc
test -f "${OUT_DIR}/compilations.kzip" || exit 1
[[ $(unzip -l "${OUT_DIR}/compilations.kzip" | grep '/files/.' | wc -l) -eq 3 ]]
[[ $(unzip -l "${OUT_DIR}/compilations.kzip" | grep '/units/.' | wc -l) -eq 1 ]]
