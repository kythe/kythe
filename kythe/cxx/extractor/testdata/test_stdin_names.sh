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
set -o pipefail
TEST_NAME="test_stdin_names"
. ./kythe/cxx/extractor/testdata/test_common.sh
rm -rf -- "${OUT_DIR}"
echo '#define STDIN_OK 1\n' | KYTHE_INDEX_PACK=1 \
    KYTHE_OUTPUT_DIRECTORY="${OUT_DIR}" \
    KYTHE_VNAMES="${BASE_DIR}/stdin.vnames" "${EXTRACTOR}" -x c -
pushd "${OUT_DIR}"
OUT_INDEX=$("${INDEXPACK}" --from_archive "${OUT_DIR}" 2>&1 | \
    sed -ne '/^.*Writing compilation unit to .*$/ { s/.*Writing compilation unit to \(.*\.kindex\)/\1/; p;}')
popd
# Make sure that the indexer can handle <stdin:> paths.
"${INDEXER}" --ignore_unimplemented=true "${OUT_DIR}/${OUT_INDEX}" | \
  "${VERIFIER}" "${BASE_DIR}/test_stdin_names_verify.cc"
