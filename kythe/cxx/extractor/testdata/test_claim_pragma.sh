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
# This test checks that the extractor handles #pragma kythe_claim.
# It should be run from the Kythe root.
TEST_NAME="test_claim_pragma"
. ./kythe/cxx/extractor/testdata/test_common.sh
EXPECTED_INDEX="701540b440f4a705d068bf22dca4963251447feb72b2cdecd24c83ac369dedf7.kindex"
INDEX_PATH="${OUT_DIR}"/"${EXPECTED_INDEX}"
rm -f -- "${INDEX_PATH}_UNIT"
KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACTOR}" --with_executable "/usr/bin/g++" \
    -I./kythe/cxx/extractor/testdata \
    ./kythe/cxx/extractor/testdata/claim_main.cc
"${KINDEX_TOOL}" -suppress_details -explode "${INDEX_PATH}"
diff "${BASE_DIR}/claim_main.UNIT" "${INDEX_PATH}_UNIT"
