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
# Test the Kythe release package for basic functionality.

export TMPDIR=${TEST_TMPDIR:?}

TEST_PORT=9898
ADDR=localhost:$TEST_PORT
TEST_REPOSRCDIR="$PWD"

jq() { "$TEST_REPOSRCDIR/third_party/jq/jq" "$@"; }

if ! command -v curl >/dev/null; then
  echo "Test requires curl command" >&2
  exit 1
fi

cd kythe/release

md5sum -c kythe-*.tar.gz.md5

rm -rf "$TMPDIR/release"
mkdir "$TMPDIR/release"
tar xzf kythe-*.tar.gz -C "$TMPDIR/release"
cd "$TMPDIR/release"

cd kythe-*

# Ensure the various tools work on test inputs
tools/viewindex "$TEST_REPOSRCDIR/kythe/testdata/test.kindex" | \
  jq . >/dev/null
tools/indexpack --to_archive indexpack.test "$TEST_REPOSRCDIR/kythe/testdata/test.kindex"
tools/entrystream < "$TEST_REPOSRCDIR/kythe/testdata/test.entries" | \
  tools/entrystream --write_json | \
  tools/entrystream --read_json --entrysets >/dev/null
tools/triples < "$TEST_REPOSRCDIR/kythe/testdata/test.entries" >/dev/null

# TODO(zarko): add cxx extractor tests
rm -rf "$TMPDIR/java_compilation"
REAL_JAVAC="$(which java)" \
  JAVAC_EXTRACTOR_JAR=$PWD/extractors/javac_extractor.jar \
  KYTHE_ROOT_DIRECTORY="$TEST_REPOSRCDIR" \
  KYTHE_OUTPUT_DIRECTORY="$TMPDIR/java_compilation" \
  KYTHE_EXTRACT_ONLY=1 \
  extractors/javac-wrapper.sh -cp "$TEST_REPOSRCDIR/third_party/guava"/*.jar \
  "$TEST_REPOSRCDIR/kythe/java/com/google/devtools/kythe/common"/*.java
cat "$TMPDIR"/javac-extractor.{out,err}
java -Xbootclasspath/p:$PWD/indexers/java_indexer.jar \
  -jar indexers/java_indexer.jar "$TMPDIR/java_compilation"/*.kindex | \
  tools/entrystream --count

# Ensure the Java indexer works on a curated test compilation
java -Xbootclasspath/p:$PWD/indexers/java_indexer.jar \
  -jar indexers/java_indexer.jar "$TEST_REPOSRCDIR/kythe/testdata/test.kindex" > entries
# TODO(zarko): add C++ test kindex entries

# Ensure basic Kythe pipeline toolset works
tools/dedup_stream < entries | \
  tools/write_entries --graphstore gs
tools/write_tables --graphstore gs --out srv

tools/read_entries --graphstore gs | \
  tools/entrystream --sort >/dev/null

# Smoke test the verifier
echo "//- Any childof Any2" > any_childof_any2
tools/verifier --ignore_dups --show_goals any_childof_any2 < entries
echo "//- Any noSuchEdge Any2" > any_nosuchedge_any2
if tools/verifier --ignore_dups any_nosuchedge_any2 < entries; then
  echo "ERROR: verifier found a non-existent edge" >&2
  exit 1
fi

# Ensure kythe tool is functional
tools/kythe --api srv nodes 'kythe:?lang=java#pkg.Names'

tools/http_server \
  --public_resources web/ui \
  --serving_table srv \
  --listen $ADDR &
pid=$!
trap "kill $pid; kill -9 $pid" EXIT ERR INT

while ! curl -s $ADDR >/dev/null; do
  echo "Waiting for http_server..."
  sleep 0.5
done

# Ensure basic HTTP handlers work
curl -sf $ADDR >/dev/null
curl -sf $ADDR/corpusRoots | jq . >/dev/null
curl -sf $ADDR/dir | jq . >/dev/null
curl -sf $ADDR/decorations -d '{"location": {"ticket": "kythe://kythe?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Names.java"}, "source_text": true, "references": true}' | \
  jq -e '(.reference | length) > 0
     and (.nodes | length) == 0
     and (.source_text | type) == "string"
     and (.source_text | length) > 0'
curl -sf $ADDR/decorations -d '{"location": {"ticket": "kythe://kythe?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Names.java"}, "references": true, "filter": ["**"]}' | \
  jq -e '(.reference | length) > 0
     and (.nodes | length) > 0
     and (.source_text | length) == 0'
