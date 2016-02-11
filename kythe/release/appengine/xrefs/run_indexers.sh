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

# Runs the Kythe analyzers over each compilation in the given directory tree.
#
# Usage INDEXERS=/opt/kythe/indexers TOOLS=/opt/kythe/tools ./run_indexers.sh /path/to/compilations

: "${INDEXERS:=/opt/kythe/indexers}"
: "${TOOLS:=/opt/kythe/tools}"
export INDEXERS TOOLS

readonly COMPILATIONS="$1"

if [[ ! -d "$COMPILATIONS" ]]; then
  echo "ERROR: $COMPILATIONS is not a directory"
  exit 1
fi

if ! command -v parallel &>/dev/null; then
  echo "ERROR: could not locate parallel"
  exit 1
fi

drive_indexer_kindex() {
  local -r lang="$(basename "$(dirname "$1")")"
  case "$lang" in
    java)
      analyzer() {
        java -jar "$INDEXERS/java_indexer.jar" "$@"
      } ;;
    c++)
      analyzer() {
        "$INDEXERS/cxx_indexer" "$@"
      } ;;
    *)
      if [[ -n "$IGNORE_UNHANDLED" ]]; then
        return 0
      fi
      echo "ERROR: no indexer found for language $lang"
      exit 1
  esac
  echo "Indexing $*" >&2
  analyzer "$@"
}
export -f drive_indexer_kindex
export SHELL=bash

cd /tmp # TODO(T70): the java indexer cannot run in the repository root
find "$COMPILATIONS" -name '*.kindex' | sort -R | \
    { parallel --gnu -L1 drive_indexer_kindex || echo "$? analysis failures" >&2; }
