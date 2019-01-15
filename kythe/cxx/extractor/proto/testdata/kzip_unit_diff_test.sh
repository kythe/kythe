#!/bin/bash
# Copyright 2018 The Kythe Authors. All rights reserved.
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
set -e

. ./kythe/cxx/extractor/proto/testdata/skip_functions.sh

KINDEX_TOOL=$1; shift
KZIP=$1; shift
GOLDEN=$1; shift

# Extract textproto version of compilation unit from kzip.
"${KINDEX_TOOL}" -canonicalize_hashes -suppress_details -explode "${KZIP}"

# Remove lines that will change depending on the machine the test is run on.
skip_inplace "-target" 1 "${KZIP}_UNIT"
skip_inplace "signature" 0 "${KZIP}_UNIT"
skip_inplace "working_directory" 0 "${KZIP}_UNIT"

echo
echo "Diffing generated unit against golden"
echo "-------------------------------------"
diff -u $GOLDEN ${KZIP}_UNIT
echo "No diffs found!"
