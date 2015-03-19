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
BASE_DIR="${PWD}/kythe/cxx/tools/fyi/testdata"
FYI="${PWD}/campfire-out/bin/kythe/cxx/tools/fyi/fyi"
OUT_DIR="${PWD}/campfire-out/test/kythe/cxx/tools/fyi/testdata/nothing_test"
HAD_ERRORS=0

mkdir -p "${OUT_DIR}"
sed "s|OUT_DIR|${OUT_DIR}|g" "${BASE_DIR}/compile_commands.json.in" > \
    "${OUT_DIR}/compile_commands.json"
cp "${BASE_DIR}/nothing.cc" "${OUT_DIR}/nothing.cc"

set +o pipefail
"${FYI}" "${OUT_DIR}/nothing.cc" | diff "${BASE_DIR}/nothing.cc.expected" -
RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[1]} )
if [ ${RESULTS[0]} -eq 0 ]; then
  >&2 echo "Expected nonzero return from tool"
  HAD_ERRORS=1
fi
if [ ${RESULTS[1]} -ne 0 ]; then
  >&2 echo "Expected zero return from diff; saw ${RESULTS[1]}."
  HAD_ERRORS=1
fi

exit ${HAD_ERRORS}
