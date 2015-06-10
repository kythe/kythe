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

BASE_DIR="$TEST_SRCDIR/kythe/cxx/indexer/cxx/testdata/rec"

one_case "${BASE_DIR}/rec_anon_struct.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/rec_class.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_base.cc" "c++1y" --ignore_dups=true
one_case "${BASE_DIR}/rec_class_base_dependent.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_base_recurring.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_header_completes.cc" "c++1y"
one_case "${BASE_DIR}/rec_class_macro.cc" "c++1y"
one_case "${BASE_DIR}/rec_struct.c" "c99"
one_case "${BASE_DIR}/rec_struct.cc" "c++1y"
one_case "${BASE_DIR}/rec_struct_decl.cc" "c++1y"
one_case "${BASE_DIR}/rec_union.cc" "c++1y"

exit ${HAD_ERRORS}
