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
#   TMP
#   LANGUAGE
#   LABEL
#   CXX_INDEXER_BIN
#   VERIFIER_BIN
#   SHOWGRAPH

SRCS="$TMP/example"
mkdir "$SRCS"
ARGS_FILE="$TMP/args"
touch "$ARGS_FILE"

# This filter assumes that its stdin is a full ObjC source file which will be
# placed into $TEST_MAIN for compilation/verification. Optionally, after the
# main source text, more files can be specified with header lines formatted like
# "#example filename".  The lines proceeding these header lines will be placed
# next to test.m in "$SRCS/filename".
TEST_MAIN="$SRCS/test.m"

# The raw filter input will be placed into this file for later syntax highlighting
RAW_EXAMPLE="$TMP/raw.hcc"

# Test entries will be dropped here.
TEST_ENTRIES="$TMP/test.entries"

# Example filter input for C++.
#   #include "test.h"
#   //- @C completes Decl1
#   //- @C completes Decl2
#   //- @C defines Defn
#   class C { };
#
#   #example test.h
#   //- @C defines Decl1
#   class C;
#   //- @C defines Decl2
#   class C;
#
# The above C++ input will generate/verify two files: test.cc and test.h

# Split collected_files.hcc into files via "#example file.name" delimiter lines.
{ echo "#example test.m";
  tee "$RAW_EXAMPLE";
} | awk -v argsfile="$ARGS_FILE" -v root="$SRCS/" '
/#example .*/ {
  x=root $2;
  next;
}

/#arguments / {
  $1 = "";
  print > argsfile;
  next;
}

{print > x;}'

CXX_ARGS="-fblocks $(cat "$ARGS_FILE")"

for TEST_M in "${SRCS}"/*.m
do
  "$CXX_INDEXER_BIN" --ignore_unimplemented=false -i "${TEST_M}" -- $CXX_ARGS \
      >> "${TEST_ENTRIES}"
done
"$VERIFIER_BIN" --ignore_dups "${SRCS}"/* < "${TEST_ENTRIES}"

trap 'error FORMAT' ERR
EXAMPLE_ID=$(sha1sum "$RAW_EXAMPLE" | cut -c 1-40)

if [[ -n "${DIV_STYLE}" ]]; then
  echo "<div style=\"${DIV_STYLE}\">"
else
  echo "<div>"
fi

echo "<h5 id=\"_${LABEL}\">${LABEL}"

if [[ "${SHOWGRAPH}" == 1 ]]; then
  "$VERIFIER_BIN" --ignore_dups --graphviz < "${TEST_ENTRIES}" > "$TMP/EXAMPLE_ID.dot"
  dot -Tsvg -o "$EXAMPLE_ID.svg" "$TMP/EXAMPLE_ID.dot"
  echo "(<a href=\"${EXAMPLE_ID}.svg\" target=\"_blank\">${LANGUAGE}</a>)</h5>"
else
  echo " (${LANGUAGE})</h5>"
fi

# I guess cpp?? Objective C doesn't seem to be in the list of possible
# langauges.
source-highlight --failsafe --output=STDOUT --src-lang cpp -i "$RAW_EXAMPLE"
echo "</div>"
