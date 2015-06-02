#!/bin/bash -e

# Copyright 2014 Google Inc. All rights reserved.
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

# This script checks that extract_compilation_database.sh works on a simple
# compilation database. It should be run from the Kythe root.
BASE_DIR="$TEST_SRCDIR/kythe/extractors/cmake"
OUT_DIR="$TEST_TMPDIR"
EXTRACT="${BASE_DIR}/extract_compilation_database.sh"
EXPECTED_INDEX="d09515d149b3ca237a31caa8fd58f48365e083ce2ebe57173d568d1939fae6b8.kindex"
EXPECTED_FILE_HASH="deac66ccb79f6d31c0fa7d358de48e083c15c02ff50ec1ebd4b64314b9e6e196"
KINDEX_TOOL="kythe/cxx/tools/kindex_tool"
rm -f "${OUT_DIR}/*.kindex*"
KYTHE_CORPUS=test_corpus KYTHE_ROOT_DIRECTORY="${BASE_DIR}/testdata" \
    KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    "${EXTRACT}" "${BASE_DIR}/testdata/compilation_database.json"
"${KINDEX_TOOL}" -suppress_details -explode "${OUT_DIR}/${EXPECTED_INDEX}"
sed "s:BASE_DIR:${BASE_DIR}:g" "${BASE_DIR}/testdata/expected.unit" \
    > "${OUT_DIR}/expected.unit"
sed "s:BASE_DIR:${BASE_DIR}:g" "${BASE_DIR}/testdata/expected.file" \
    > "${OUT_DIR}/expected.file"
diff "${OUT_DIR}/expected.unit" "${OUT_DIR}/${EXPECTED_INDEX}_UNIT"
diff "${OUT_DIR}/expected.file" "${OUT_DIR}/${EXPECTED_INDEX}_${EXPECTED_FILE_HASH}"
