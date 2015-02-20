#!/bin/bash -e

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

# Script to extract compilations from a campfire-based repository.
#
# Usage:
#   extract.sh [--docker] [--index_pack] [campfire-root] <output-dir>

usage() {
  echo "Usage: $(basename "$0") [--docker] [--index_pack] [campfire-root] <output-dir>" >&2
  exit 1
}

CAMPFIRE=./campfire
if [[ "$1" == "--docker" ]]; then
  CAMPFIRE=./campfire-docker
  shift
fi

INDEX_PACK=
if [[ "$1" == "--index_pack" ]]; then
  INDEX_PACK=1
  shift
fi

ROOT=.
case $# in
  1)
    OUTPUT="$1" ;;
  2)
    ROOT="$1"
    OUTPUT="$2" ;;
  *)
    usage ;;
esac

mkdir -p "$OUTPUT"
OUTPUT="$(readlink -e "$OUTPUT")"

cd "$ROOT"
"$CAMPFIRE" extract

for idx in $(find campfire-out/gen -name '*.kindex'); do
  name="$(basename "$idx" .kindex)"
  lang="${name##*.}"
  dir="$OUTPUT/$lang"

  if [[ -n "$INDEX_PACK" ]]; then
      ./campfire-out/bin/kythe/go/platform/tools/indexpack --to_archive "$dir" "$idx"
  else
    mkdir -p "$dir"
    dest="$dir/$(tr '/' '_' <<<"${idx#campfire-out/gen/}")"
    cp "$idx" "$dest"
  fi
done
