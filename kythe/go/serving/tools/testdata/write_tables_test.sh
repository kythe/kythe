#!/bin/bash -e
set -o pipefail
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

jq=third_party/jq/jq
root=kythe/go/serving/tools/testdata

echo "Building new serving table"
$root/entries2tables kythe/testdata/entries.gz "$TEST_TMPDIR/serving_table"

echo "Splitting serving data in JSON"
$root/debug_serving.sh "$TEST_TMPDIR/serving_table"

check_diff() {
  local table="serving_table.$1.json"
  local gold="kythe/testdata/$table"
  local new="$TEST_TMPDIR/$table"
  gzip -kdf "$gold.gz"

  echo
  if ! diff -q "$gold" "$new"; then
    $jq .key "$gold" > "$TEST_TMPDIR/gold.keys"
    $jq .key "$new" > "$TEST_TMPDIR/new.keys"

    if ! diff -u "$TEST_TMPDIR/"{gold,new}.keys | diffstat -qm; then
      echo "  Key samples:"
      echo "    Unique to gold:"
      comm -23 "$TEST_TMPDIR/"{gold,new}.keys | sort -R | head -n3
      echo "    Unique to new:"
      comm -13 "$TEST_TMPDIR/"{gold,new}.keys | sort -R | head -n3
    fi

    echo "  Key-value samples:"
    echo "    Unique to gold:"
    comm -23 <($jq -S -c . "$gold") <($jq -S -c . "$new") | \
      sort -R | head -n1 | $jq .
    echo "    Unique to new:"
    comm -13 <($jq -S -c . "$gold") <($jq -S -c . "$new") | \
      sort -R | head -n1 | $jq .

    return 1
  fi
}

fail=

echo "Testing for differences"
check_diff decor || fail=1
check_diff xrefs || fail=1
check_diff xrefPages || fail=1
check_diff edgeSets || fail=1
check_diff edgePages || fail=1

if [[ -n "$fail" ]]; then
  exit 1
fi
