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

VERIFIER="kythe/cxx/verifier/verifier"
INDEXER="kythe/cxx/indexer/cxx/indexer"
# one_case test-file clang-standard indexer-argument1 indexer-argument2
#          verifier-argument1 verifier-argument2
#          [expectfailindex | expectfailverify]
${INDEXER} -i $1 $3 $4 -- -std=$2 | ${VERIFIER} $1 $5 $6
RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[1]} )
if [ ${RESULTS[1]} -eq 2 ]; then
  echo "[ BAD VERIFIER SCRIPT: $1 ]"
  exit 1
elif [ ${RESULTS[0]} -ne 0 ]; then
  echo "[ FAILED INDEX: $1 ]"
  if [ "$7" == 'expectfailindex' ]; then
    exit 0
  fi
elif [ ${RESULTS[1]} -ne 0 ]; then
  echo "[ FAILED VERIFY: $1 ]"
  if [ "$7" == 'expectfailverify' ]; then
    exit 0
  fi
else
  echo "[ OK: $1 ]"
  if [ -z "$7" ]; then
    exit 0
  fi
fi
exit 1
