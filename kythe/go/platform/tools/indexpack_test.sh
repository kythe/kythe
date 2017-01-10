#!/bin/bash -e
#
# Copyright 2014 Google Inc. All rights reserved.
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
# This script tests the indexpack binary. It requires the jq command (≥ 1.4).

jq="third_party/jq/jq"
indexpack="kythe/go/platform/tools/indexpack/indexpack"
viewindex="kythe/go/platform/tools/viewindex/viewindex"
test_kindex="$PWD/kythe/testdata/test.kindex"

kindex_contents() {
  $viewindex --files "$1" | $jq -c -S .
}

tmp="$(mktemp -d 2>/dev/null || mktemp -d -t 'kythetest')"
trap 'rm -rf "$tmp"' EXIT ERR INT

$indexpack --to_archive "$tmp/archive" "$test_kindex" >/dev/null
$indexpack --from_archive "$tmp/archive" "$tmp/idx"

kindex_file="$(find "$tmp/idx" -name "*.kindex")"

result="$(kindex_contents "$kindex_file")"
expected="$(kindex_contents $test_kindex)"

if [[ ! "$result" == "$expected" ]]; then
  echo "ERROR: expected $expected; received $result" >&2
  exit 1
fi
