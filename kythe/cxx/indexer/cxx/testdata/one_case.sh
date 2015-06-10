#!/bin/bash

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

# This script defines one_case, a function that runs the indexer and verifier
# on a single file test case. The test case contains assertions for the
# verifier to verify. If a test fails, the variable HAD_ERRORS will be set to 1.

HAD_ERRORS=0
VERIFIER="kythe/cxx/verifier/verifier"
INDEXER="kythe/cxx/indexer/cxx/indexer"
# one_case test-file clang-standard verifier-argument indexer-argument
function one_case {
  ${INDEXER} -i $1 $4 $5 -- -std=$2 | ${VERIFIER} $1 $3
  RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[1]} )
  if [ ${RESULTS[0]} -ne 0 ]; then
    echo "[ FAILED INDEX: $1 ]"
    HAD_ERRORS=1
  elif [ ${RESULTS[1]} -ne 0 ]; then
    echo "[ FAILED VERIFY: $1 ]"
    HAD_ERRORS=1
  else
    echo "[ OK: $1 ]"
  fi
  return $HAD_ERRORS
}
