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

# This script runs the indexer on various test cases, piping the results
# to the verifier. The test cases contain assertions for the verifier to
# verify. Should every case succeed, this script returns zero.
HAD_ERRORS=0
KYTHE_BIN="${TEST_SRCDIR:-${PWD}/campfire-out/bin}"
BASE_DIR="${TEST_SRCDIR:-${PWD}}/kythe/cxx/indexer/cxx/testdata/libraries"
VERIFIER="${KYTHE_BIN}/kythe/cxx/verifier/verifier"
INDEXER="${KYTHE_BIN}/kythe/cxx/indexer/cxx/indexer"
# one_case test-file clang-standard verifier-argument indexer-argument
function one_case {
  ${INDEXER} -i $1 $4 -- -std=$2 | ${VERIFIER} $1 $3
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
}

# Duplicates appear to come from refs in the source file at definition macros.
# The refs are to internal definitions (for example, in flags_ref_int64_defn,
# the duplicate is a ref to FLAGS_nonodefnflag).
one_case "${BASE_DIR}/flags_bool.cc" "c++1y" -ignore_dups
one_case "${BASE_DIR}/flags_decls.cc" "c++1y"
one_case "${BASE_DIR}/flags_defns.cc" "c++1y" -ignore_dups
one_case "${BASE_DIR}/flags_empty.cc" "c++1y"
one_case "${BASE_DIR}/flags_ref_int64_decl.cc" "c++1y"
one_case "${BASE_DIR}/flags_ref_int64_defn.cc" "c++1y" -ignore_dups
one_case "${BASE_DIR}/flags_ref_int64_defn_completes.cc" "c++1y" -ignore_dups
one_case "${BASE_DIR}/flags_string.cc" "c++1y" -ignore_dups

exit ${HAD_ERRORS}
