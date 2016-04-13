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
# test_case.sh test_basename.cc
# runs a single fyi test, expecting the following files to exist:
#   test_basename.cc -- input source file
#   test_basename.cc.expected -- expected output
#   test_basename.cc.json -- input kythe database
#   compile_commands.json.in -- shared compilation database
#     will substitute "OUT_DIR" for the test output directory

set -o pipefail
TEST_NAME="$1"
BASE_DIR="$PWD/kythe/cxx/tools/fyi/testdata"
TEST_JSON="${BASE_DIR}/${TEST_NAME}.json"
TEST_EXPECTED="${BASE_DIR}/${TEST_NAME}.expected"
FYI="kythe/cxx/tools/fyi/fyi"
OUT_DIR="$TEST_TMPDIR"
TEST_FILE="${OUT_DIR}/${TEST_NAME}"
HAD_ERRORS=0

source "kythe/cxx/common/testdata/start_http_service.sh"

mkdir -p "${OUT_DIR}"
sed "s|OUT_DIR|${OUT_DIR}|g
s|BASE_DIR|${BASE_DIR}|g" "${BASE_DIR}/compile_commands.json.in" > \
    "${OUT_DIR}/compile_commands.json"
cp "${BASE_DIR}/${TEST_NAME}" "${TEST_FILE}"

set +e
"${FYI}" --xrefs="${XREFS_URI}" "${TEST_FILE}" > "${TEST_FILE}.actual" 
RESULTS=$?
set -e

if [[ -e "${TEST_EXPECTED}" ]]; then
  if [[ ${RESULTS} -ne 0 ]]; then
    >&2 echo "Expected zero return from tool, saw ${RESULTS}"
    HAD_ERRORS=1
  fi
  diff "${TEST_FILE}.actual" "${TEST_EXPECTED}"
else
  if [[ ${RESULTS} -eq 0 ]]; then
    >&2 echo "Expected nonzero return from tool"
    HAD_ERRORS=1
  fi
fi

if [[ ${HAD_ERRORS} -ne 0 ]]; then
  echo gdb --args "${FYI}" --xrefs="${XREFS_URI}" "${TEST_FILE}" >&2
fi

exit ${HAD_ERRORS}
