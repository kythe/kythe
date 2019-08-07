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

KZIP_TOOL=$1; shift
JQ=$1; shift
KZIP=$1; shift
GOLDEN=$1; shift

# Work in tmpdir because the runfiles dir is not writable
cp $KZIP "${TEST_TMPDIR}/file.kzip"
# Extract textproto version of compilation unit from kzip.
UNIT=$("${KZIP_TOOL}" view "${TEST_TMPDIR}/file.kzip")
UNIT_FILE="${TEST_TMPDIR}/$(basename $GOLDEN)"
# Remove working_directory, which will change depending on the machine the test
# is run on.
echo "$UNIT" | $JQ 'del(.working_directory)' > "$UNIT_FILE"

echo
echo "Diffing generated unit against golden"
echo "-------------------------------------"
diff -u $GOLDEN $UNIT_FILE
echo "No diffs found!"
