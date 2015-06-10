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

. "${TEST_SRCDIR}/kythe/cxx/indexer/cxx/testdata/one_case.sh"

BASE_DIR="$TEST_SRCDIR/kythe/cxx/indexer/cxx/testdata/libraries"

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
