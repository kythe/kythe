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

# entries2tables.sh runs write_tables over a given entries file.
#
# Usage: entries2tables.sh <path-to-entries> <path-to-output-leveldb>

ENTRIES="$1"
OUT="${2:?"ERROR: please specify out directory"}"

if [[ ! -r "$ENTRIES" ]]; then
  echo "ERROR: unable to read entries at \"$ENTRIES\"" >&2
  exit 1
elif [[ -e "$OUT" ]]; then
  echo "ERROR: \"$OUT\" already exists" >&2
  exit 1
fi

entrystream=kythe/go/platform/tools/entrystream/entrystream
write_tables=kythe/go/serving/tools/write_tables/write_tables

if [[ ! -x "$entrystream" || -d "$entrystream" ]]; then
  entrystream="$(which entrystream)"
fi
if [[ ! -x "$write_tables" || -d "$write_tables" ]]; then
  write_tables="$(which write_tables)"
fi


mkdir -p "$(dirname "$OUT")"

cat=cat
if [[ "$ENTRIES" == *.gz ]]; then
  cat="gunzip -c"
fi

$cat "$ENTRIES" | \
  "$entrystream" --unique | \
  "$write_tables" --max_page_size 75 --entries - --out "$OUT"
