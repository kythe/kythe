#!/bin/bash
set -eo pipefail

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
#
# Test the Kythe release package for basic functionality.

export TMPDIR=${TEST_TMPDIR:?}
SHASUM_TOOL="$PWD/$1"
shift

TEST_PORT=9898
ADDR=localhost:$TEST_PORT
TEST_REPOSRCDIR="$PWD"

# The Java extractor and indexer need access to various packages in the jdk.compiler module.
JAVA_EXPORTS=(
  jdk.compiler/com.sun.tools.javac.api
  jdk.compiler/com.sun.tools.javac.code
  jdk.compiler/com.sun.tools.javac.file
  jdk.compiler/com.sun.tools.javac.main
  jdk.compiler/com.sun.tools.javac.model
  jdk.compiler/com.sun.tools.javac.parser
  jdk.compiler/com.sun.tools.javac.tree
  jdk.compiler/com.sun.tools.javac.util
)

export KYTHE_JAVA_RUNTIME_OPTIONS="${JAVA_EXPORTS[*]/*/--add-exports=&=ALL-UNNAMED}"

jq() { "$TEST_REPOSRCDIR/external/com_github_stedolan_jq/jq" "$@"; }

if ! command -v curl > /dev/null; then
  echo "Test requires curl command" >&2
  exit 1
fi

cd kythe/release

EXPECTED_SUM=$(cat kythe-*.tar.gz.sha256)
SUM=$("$SHASUM_TOOL" kythe-*.tar.gz)
if [[ "$SUM" != "$EXPECTED_SUM" ]]; then
  echo "Expected digest \"$EXPECTED_SUM\" but got \"$SUM\"."
  exit 1
fi

rm -rf "$TMPDIR/release"
mkdir "$TMPDIR/release"
tar xzf kythe-*.tar.gz -C "$TMPDIR/release"
cd "$TMPDIR/release"

cd kythe-*

# Ensure the various tools work on test inputs
tools/kzip view "$TEST_REPOSRCDIR/kythe/testdata/test.kzip" |
  jq . > /dev/null
tools/entrystream < "$TEST_REPOSRCDIR/kythe/testdata/test.entries" |
  tools/entrystream --write_format=json |
  tools/entrystream --read_format=json --entrysets > /dev/null
tools/triples < "$TEST_REPOSRCDIR/kythe/testdata/test.entries" > /dev/null

# TODO(zarko): add cxx extractor tests

rm -rf "$TMPDIR/java_compilation"
export KYTHE_OUTPUT_FILE="$TMPDIR/java_compilation/util.kzip"
JAVAC_EXTRACTOR_JAR=$PWD/extractors/javac_extractor.jar \
  KYTHE_ROOT_DIRECTORY="$TEST_REPOSRCDIR" \
  KYTHE_EXTRACT_ONLY=1 \
  extractors/javac-wrapper.sh \
  "$TEST_REPOSRCDIR/kythe/java/com/google/devtools/kythe/util"/*.java
test -r "$KYTHE_OUTPUT_FILE"
# We do not want to inhibit word splitting here.
# shellcheck disable=SC2086
java $KYTHE_JAVA_RUNTIME_OPTIONS \
  -jar indexers/java_indexer.jar "$KYTHE_OUTPUT_FILE" |
  tools/entrystream --count

# Ensure the Java indexer works on a curated test compilation
# We do not want to inhibit word splitting here.
# shellcheck disable=SC2086
java $KYTHE_JAVA_RUNTIME_OPTIONS \
  -jar indexers/java_indexer.jar "$TEST_REPOSRCDIR/kythe/testdata/test.kzip" > entries
# TODO(zarko): add C++ test kzip entries

# Ensure basic Kythe pipeline toolset works
tools/dedup_stream < entries |
  tools/write_entries --graphstore gs
tools/write_tables --graphstore gs --out srv

tools/read_entries --graphstore gs |
  tools/entrystream --sort > /dev/null

# Smoke test the verifier
echo "//- _ childof _" > any_childof_any2
tools/verifier --nofile_vnames --ignore_dups --show_goals any_childof_any2 \
  < entries
echo "//- _ noSuchEdge _" > any_nosuchedge_any2
if tools/verifier --nofile_vnames --ignore_dups any_nosuchedge_any2 < entries; then
  echo "ERROR: verifier found a non-existent edge" >&2
  exit 1
fi

# Ensure kythe tool is functional
tools/kythe --api srv nodes 'kythe:?lang=java#pkg.Names'
tools/http_server \
  --serving_table srv \
  --listen $ADDR &
pid=$!
trap "kill $pid; kill -9 $pid" EXIT ERR INT

while ! curl -s $ADDR > /dev/null; do
  echo "Waiting for http_server..."
  sleep 0.5
done

# Ensure basic HTTP handlers work
curl -sf $ADDR/corpusRoots | jq . > /dev/null
curl -sf $ADDR/dir | jq . > /dev/null
curl -sf $ADDR/decorations -d '{"location": {"ticket": "kythe://kythe?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Names.java"}, "source_text": true, "references": true}' |
  jq -e '(.reference | length) > 0
     and (.nodes | length) == 0
     and (.source_text | type) == "string"
     and (.source_text | length) > 0'
curl -sf $ADDR/decorations -d '{"location": {"ticket": "kythe://kythe?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Names.java"}, "references": true, "filter": ["**"]}' |
  jq -e '(.reference | length) > 0
     and (.nodes | length) > 0
     and (.source_text | length) == 0'
