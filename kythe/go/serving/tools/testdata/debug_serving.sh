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

# debug_serving.sh splits Kythe LevelDB serving data into separate JSON files
# per table section (e.g. decor, edgeSets, edgePages, etc.).
#
# Usage: debug_serving.sh <path-to-leveldb>

TABLE="$1"

if [[ ! -d "$TABLE" ]]; then
  echo "ERROR: unable to read serving data at \"$TABLE\"" >&2
  exit 1
fi

scan_leveldb=kythe/go/util/tools/scan_leveldb/scan_leveldb
jq=third_party/jq/jq

if [[ ! -x "$scan_leveldb" || -d "$scan_leveldb" ]]; then
  scan_leveldb="$(which scan_leveldb)"
fi
if [[ ! -x "$jq" || -d "$jq" ]]; then
  jq="$(which jq)"
fi

scan() {
  local out="$TABLE.${1%:}.json"
  echo -n "$out: " >&2
  "$scan_leveldb" --prefix $1 --proto_value $2 --json "$TABLE" | \
    tee >(wc -l >&2) | \
    "$jq" -S . > "$out"
}

scan edgeSets:  kythe.proto.serving.PagedEdgeSet
scan edgePages: kythe.proto.serving.EdgePage
scan xrefs:     kythe.proto.serving.PagedCrossReferences
scan xrefPages: kythe.proto.serving.PagedCrossReferences.Page
scan decor:     kythe.proto.serving.FileDecorations
