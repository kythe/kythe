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

# This script verifies and formats a single Kythe example, which is expected
# to be piped in on standard input from example.sh.
#
# The script assumes its working directory is the schema output directory and
# requires the following environment variables:
#   GOROOT
#   TMP
#   LANGUAGE
#   LABEL
#   GO_INDEXER_BIN
#   VERIFIER_BIN
#   SHOWGRAPH

readonly PKGDIR="$TMP/pkg"
readonly SRCFILE="$PKGDIR/example.go"
mkdir "$PKGDIR"
cat > "$SRCFILE"

readonly ENTRIES="$TMP/example.entries"
"$GO_INDEXER_BIN" --package kythe/schema "$SRCFILE" > "$ENTRIES"
"$VERIFIER_BIN" --use_file_nodes < "$ENTRIES"

trap 'error FORMAT' ERR
readonly EXAMPLE_ID=$(sha1sum "$SRCFILE" | cut -c 1-40)

if [[ -n "$DIV_STYLE" ]] ; then
  echo "<div style=\"${DIV_STYLE}\">"
else
  echo "<div>"
fi

echo "<h5 id=\"_${LABEL}\">${LABEL}"

if [[ "${SHOWGRAPH}" == 1 ]] ; then
  "$VERIFIER_BIN" --use_file_nodes --graphviz \
    < "$ENTRIES" > "$TMP/${EXAMPLE_ID}.dot"
  dot -Tsvg -o "${EXAMPLE_ID}.svg" "$TMP/${EXAMPLE_ID}.dot"
  echo "(<a href=\"${EXAMPLE_ID}.svg\" target=\"_blank\">${LANGUAGE}</a>)</h5>"
else
  echo " (${LANGUAGE})</h5>"
fi

source-highlight --failsafe --output=STDOUT --src-lang go -i "$SRCFILE"
echo "</div>"

