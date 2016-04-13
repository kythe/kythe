#!/bin/bash -e
set -o pipefail
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
#
# This script checks for the existance of https://kythe.io/phabricator/T70

indexer="kythe/java/com/google/devtools/kythe/analyzers/java/indexer"
entrystream="kythe/go/platform/tools/entrystream"
test_kindex="$PWD/kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/corner_case.kindex"

# This will emit an error if https://kythe.io/phabricator/T70 is not solved.
"$indexer" "$test_kindex" 2>"$TEST_TMPDIR/err.log" | \
  echo "INFO: entrystream read $("$entrystream" --count) entries"

if grep -qE 'Exception|error' "$TEST_TMPDIR/err.log"; then
  echo "ERROR while indexing $test_kindex" >&2
  cat "$TEST_TMPDIR/err.log" >&2
  exit 1
fi
