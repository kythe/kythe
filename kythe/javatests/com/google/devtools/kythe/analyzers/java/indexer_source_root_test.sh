#!/bin/bash
set -eo pipefail
# Copyright 2015 The Kythe Authors. All rights reserved.
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
#
# This script checks for the existance of #818

: "${indexer?:missing indexer}"
: "${entrystream?:missing entrystream}"
test_kzip="$PWD/kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/corner_case.kzip"

# This line removes the precondition for #818 (allowing the test to pass).
# find -L -name KytheEntrySets.java -delete

# This will emit an error if https://kythe.io/phabricator/T70 is not solved.
"$indexer" "$test_kzip" 2>"$TEST_TMPDIR/err.log" | \
  echo "INFO: entrystream read $("$entrystream" --count) entries"

if grep -qE 'Exception|error|KytheEntrySets not seen during extraction' "$TEST_TMPDIR/err.log"; then
  echo "ERROR while indexing $test_kzip" >&2
  cat "$TEST_TMPDIR/err.log" >&2
  exit 1
fi
