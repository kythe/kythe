#!/bin/bash
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

# Script to package a release tar and create its associated .sha256 checksum.
#
# Usage: package_release.sh <shasum_tool> <path-to-output-tar.gz> [package contents]
#
# In the simplest case, each file given will be placed in the root of the
# resulting archive.  The --relpath, --path, and --cp flags change this behavior
# so that file paths can be structured.
#
#   --verbose: Log each action as it is taken.
#   --path <path>: Each file is copied to ARCHIVE_ROOT/<path>/$(basename file).
#   --relpaths <prefix>: Strip $GENBIR, $BINDIR and then <prefix> from each
#                        file's path.  The resulting path is used for the file
#                        inside of the archive.  This combines with --path to
#                        change the root of the resulting file path.
#   --cp <path> <path>: Copy the first file to the archive using exactly the
#                       second path.
#   --exclude <glob>: Exclude all further files matching the given glob.
#
# Example:
#   BINDIR=bazel-bin/ \
#     package_release.sh /path/to/sha /tmp/b.tar.gz README.adoc LICENSE \
#       --path some/path/for/docs kythe/docs/kythe-{overview,storage}.txt \
#       --relpaths kythe/docs bazel-bin/kythe/docs/schema/schema.html \
#       --cp CONTRIBUTING.md kythe/docs/how-to-contribute.md
#
#   Resulting tree in /tmp/b.tar.gz:
#     README.adoc
#     LICENSE
#     kythe/docs/
#       kythe-overview.txt
#       kythe-storage.txt
#       schema.html
#       how-to-contribute.md
set -e

SHASUM_TOOL="$1"
shift

OUT="$1"
shift

PBASE="$OUT.dir/$(basename "$OUT" .tar.gz)"
P=$PBASE

mkdir -p "$PBASE"
# shellcheck disable=SC2064
trap "rm -rf '$PWD/$OUT.dir'" EXIT ERR INT

VERBOSE=
function log() {
  if [[ -z "$VERBOSE" ]]; then
    return
  fi
  echo "$@" >&2
}

EXCLUDE=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    --verbose)
      VERBOSE=true
      ;;
    --relpaths)
      RELPATHS=$2
      shift
      ;;
    --path)
      P="$PBASE/$2"
      mkdir -p "$P"
      shift
      ;;
    --cp)
      mkdir -p "$PBASE/$(dirname "$3")"
      cp "$2" "$PBASE/$3"
      shift 2
      ;;
    --exclude)
      EXCLUDE+=("$2")
      shift
      ;;
    *)
      skip=
      for exclude in "${EXCLUDE[@]}"; do
        if [[ "$1" =~ $exclude ]]; then
          skip=true
          break
        fi
      done
      if [[ -n "$skip" ]]; then
        log "Excluding $1"
      elif [[ -z "$RELPATHS" ]]; then
        log "Copying $1 to $P"
        cp "$1" "$P"/
      else
        rp="${1#"$GENDIR"/}"
        rp="${rp#"$BINDIR"/}"
        rp="$(dirname "${rp#"$RELPATHS"/}")"
        mkdir -p "$P/$rp"
        log "Copying $1 to $P/$rp"
        cp "$1" "$P/$rp"
      fi
      ;;
  esac
  shift
done

tar czf "$OUT" -C "$OUT.dir" "$(basename "$PBASE")"

$SHASUM_TOOL "$OUT" > "$OUT.sha256"

echo "Release outputs are ${OUT} and ${OUT}.sha256"
