#!/bin/bash -e
#
# Copyright 2018 The Kythe Authors. All rights reserved.
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
# Download, build, and extract a set of Go packages.  Resulting compilations
# will be merged into a single .kzip archive in the "$OUTPUT" directory.

: "${TMPDIR:=/tmp}" "${OUTPUT:=/output}"

FLAGS=()
PACKAGES=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    -*)
      FLAGS+=("$1") ;;
    *)
      PACKAGES+=("$1") ;;
  esac
  shift
done

# golang build tags can optionally be specified with the `KYTHE_GO_BUILD_TAGS`
# env variable.
if [ ! -z "$KYTHE_GO_BUILD_TAGS" ]; then
  FLAGS+=( "--buildtags=$KYTHE_GO_BUILD_TAGS" )
fi

if [ -n "$KYTHE_PRE_BUILD_STEP" ]; then
  eval "$KYTHE_PRE_BUILD_STEP"
fi

echo "Downloading ${PACKAGES[*]}" >&2
go get -d "${PACKAGES[@]}" || true

echo "Extracting ${PACKAGES[*]}" >&2
parallel --will-cite \
  extract_go --continue -v \
  --goroot="$(go env GOROOT)" \
  --output="$TMPDIR/out.{#}.kzip" \
  "${FLAGS[@]}" \
  {} ::: "${PACKAGES[@]}"

mkdir -p "$OUTPUT"
OUT="$OUTPUT/compilations.kzip"
if [[ -f "$OUT" ]]; then
  OUT="$(mktemp -p "$OUTPUT/" compilations.XXXXX.kzip)"
fi
echo "Merging compilations into $OUT" >&2
kzip merge --output "$OUT" "$TMPDIR"/out.*.kzip
fix_permissions.sh "$OUTPUT"
