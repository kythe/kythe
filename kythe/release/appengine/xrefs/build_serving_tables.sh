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
# Builds serving tables based on either Kythe's sources, a set of compilation
# units, or a populated GraphStore.
#
# Usage: ./build_serving_tables.sh [--index_files] [--graphstore gs] [--compilations dir] [--out path]
#
# The default --out directory is ./serving.  This script is expected to be in
# the ./kythe/release/appengine/xrefs directory of the Kythe repository.

: "${INDEXERS:=/opt/kythe/indexers}"
: "${TOOLS:=/opt/kythe/tools}"

echo "TOOLS=$TOOLS"
echo "INDEXERS=$INDEXERS"

COMPILATIONS=
GRAPHSTORE=
TABLES="$PWD/serving"
INDEX_FILES=

while [[ $# -gt 0 ]]; do
  case $1 in
    --index_files)
      INDEX_FILES=1 ;;
    --graphstore)
      GRAPHSTORE="$(readlink -m "$2")"
      shift ;;
    --compilations)
      COMPILATIONS="$(readlink -m "$2")"
      shift ;;
    --out)
      TABLES="$(readlink -m "$2")"
      shift ;;
    *)
      echo "Unknown argument: '$1'" >&2
      exit 1 ;;
  esac
  shift
done

cd "$(dirname "$0")/../../../.."

if [[ -z "$GRAPHSTORE" ]]; then
  GRAPHSTORE="$(mktemp -d --suffix=.kythe_graphstore)"
else
  mkdir -p "$GRAPHSTORE"
fi

if [[ -n "$(find "$GRAPHSTORE" -maxdepth 0 -empty)" ]]; then
  if [[ -z "$COMPILATIONS" ]]; then
    COMPILATIONS="$(mktemp -d --suffix=.kythe_compilations)"
  else
    mkdir -p "$COMPILATIONS"
  fi

  if [[ -z "$(find "$COMPILATIONS" -name '*.kindex' -print -quit)" ]]; then
    echo "Extracting Kythe compilations to $COMPILATIONS" >&2
    time ./kythe/extractors/bazel/extract.sh "$PWD" "$COMPILATIONS"
  fi

  echo "Writing to GraphStore at $GRAPHSTORE" >&2
  "$(dirname "$0")/run_indexers.sh" "$COMPILATIONS" | \
    "$TOOLS/dedup_stream" | \
    "$TOOLS/write_entries" --graphstore "$GRAPHSTORE"

  if [[ -n "$INDEX_FILES" ]]; then
    TMP_REPO="$(mktemp -d)"
    trap "rm -rf '$TMP_REPO'" EXIT ERR INT
    COMMIT=$(git rev-parse --abbrev-ref HEAD)
    echo "Checking out $COMMIT to $TMP_REPO for file indexing" >&2
    git --work-tree="$TMP_REPO" checkout -f $COMMIT

    cd "$TMP_REPO"
    "$TOOLS/directory_indexer" --vnames kythe/data/vnames.json | \
      "$TOOLS/write_entries" --graphstore "$GRAPHSTORE"
  fi
fi

echo "Writing serving tables to $TABLES"
rm -rf "$TABLES"
mkdir -p "$TABLES"
time "$TOOLS/write_tables" --graphstore "$GRAPHSTORE" --out "$TABLES"
