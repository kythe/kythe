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

echo "Downloading $*" >&2
go get -d "$@" || true

echo "Extracting $*" >&2
parallel --will-cite \
  extract_go --continue -v \
  --goroot="$(go env GOROOT)" \
  --output="$TMPDIR/out.{#}.kzip" \
  {} ::: "$@"

mkdir -p "$OUTPUT"
OUT="$(mktemp -p "$OUTPUT/" compilations.XXXXX.kzip)"
echo "Merging compilations into $OUT" >&2
zipmerge "$OUT" "$TMPDIR"/out.*.kzip
