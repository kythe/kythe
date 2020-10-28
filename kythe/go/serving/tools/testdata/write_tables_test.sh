#!/bin/bash
set -eo pipefail
# Copyright 2016 The Kythe Authors. All rights reserved.
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

readonly TABLE="$TEST_TMPDIR/serving_table"

scan() {
  local out="$TABLE.${1%:}.json"
  echo -n "$out: " >&2
  "$scan_leveldb" --prefix $1 --proto_value $2 --json "$TABLE" | \
    tee >(wc -l >&2) | \
    "$jq" -S . > "$out"
}

entries2tables() {
  local ENTRIES="$1"
  local OUT="$2"

  gunzip -c "$ENTRIES" | \
    "$entrystream" --unique | \
    "$write_tables" --max_page_size 75 --entries - --out "$OUT"
}

echo "Building new serving table"
entries2tables kythe/testdata/entries.gz "$TABLE"

echo "Splitting serving data in JSON"
scan edgeSets:  kythe.proto.serving.PagedEdgeSet
scan edgePages: kythe.proto.serving.EdgePage
scan xrefs:     kythe.proto.serving.PagedCrossReferences
scan xrefPages: kythe.proto.serving.PagedCrossReferences.Page
scan decor:     kythe.proto.serving.FileDecorations

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
