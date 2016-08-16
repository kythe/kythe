#!/bin/bash

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

VERIFIER="kythe/cxx/verifier/verifier"
INDEXER="rustc -L kythe/rust/indexer/ -Z extra-plugins=kythe_indexer -Z no-trans"
KYTHE_ENTRY_STREAM="kythe/go/platform/tools/entrystream/entrystream"

export KYTHE_CORPUS=""

source kythe/rust/indexer/testdata/parse_args.sh
echo "TEST_FILE = ${TEST_FILE}"
echo "CRATE_TYPE = ${CRATE_TYPE}"
${INDEXER} --crate-type=${CRATE_TYPE} "${TEST_FILE}" | \
    "${KYTHE_ENTRY_STREAM}" --read_json | \
    "${VERIFIER}" "${TEST_FILE}" "${VERIFIER_ARGS[@]}"

RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[2]} )
source kythe/rust/indexer/testdata/handle_results.sh
