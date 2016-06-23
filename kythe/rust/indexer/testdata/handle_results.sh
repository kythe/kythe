# Copyright 2016 Google Inc. All rights reserved.
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

# handle_results should be inlined into another script.
# It expects $RESULTS to be set to an array where the first element is the
# return code from the indexer and the second is the return code from the
# verifier.
# It expects $RESULTS_EXPECTED to be set to one of
# 'expectfailindex', 'expectfailverify', or ''.
# It expects $TEST_FILE to be set to the name (or some description of)
# the test file.

if [ ${RESULTS[1]} -eq 2 ]; then
  echo "[ BAD VERIFIER SCRIPT: ${TEST_FILE} ]"
  exit 1
elif [ ${RESULTS[0]} -ne 0 ]; then
  echo "[ FAILED INDEX: ${TEST_FILE} ]"
  if [ "${RESULTS_EXPECTED}" == 'expectfailindex' ]; then
    exit 0
  fi
elif [ ${RESULTS[1]} -ne 0 ]; then
  echo "[ FAILED VERIFY: ${TEST_FILE} ]"
  if [ "${RESULTS_EXPECTED}" == 'expectfailverify' ]; then
    exit 0
  fi
else
  echo "[ OK: ${TEST_FILE} ]"
  if [ -z "${RESULTS_EXPECTED}" ]; then
    exit 0
  fi
fi
exit 1
