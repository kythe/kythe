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

TEST_PORT=9898
ADDR=localhost:$TEST_PORT

jq() { "$TEST_SRCDIR/third_party/jq/jq" "$@"; }

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
tools/viewindex "$TEST_SRCDIR/kythe/testdata/test.kindex" | \
  jq . >/dev/null
tools/indexpack --to_archive indexpack.test "$TEST_SRCDIR/kythe/testdata/test.kindex"
tools/entrystream < "$TEST_SRCDIR/kythe/testdata/test.entries" | \
  tools/entrystream --write_json >/dev/null
tools/triples < "$TEST_SRCDIR/kythe/testdata/test.entries" >/dev/null

# TODO(schroederc): add extractor tests

# Ensure the Java indexer works on a curated test compilation
java -jar indexers/java_indexer.jar "$TEST_SRCDIR/kythe/testdata/test.kindex" > entries
# TODO(zarko): add C++ test kindex entries

# Ensure basic Kythe pipeline toolset works
tools/dedup_stream < entries | \
  tools/write_entries --graphstore gs
tools/write_tables --graphstore gs --out srv

tools/read_entries --graphstore gs | \
  tools/entrystream --sort >/dev/null

# Smoke test the verifier
tools/verifier --ignore_dups --show_goals <(echo "//- Any childof Any2") < entries
if tools/verifier --ignore_dups <(echo "//- Any noSuchEdge Any2") < entries; then
  echo "ERROR: verifier found a non-existent edge" >&2
  exit 1
fi

# Ensure kythe tool is functional
tools/kythe --api srv search /kythe/node/kind file | \
  grep -q 'kythe://kythe?lang=java?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Names.java#7571dfe62c239daa2caaeed97638184533b0526f4ab16c872311c954100d11e3'
tools/kythe --api srv node 'kythe:?lang=java#pkg.Names'

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
curl -sf $ADDR/search -d '{"partial": {"language": "java"}, "fact": [{"name": "/kythe/node/kind", "value": "cmVjb3Jk"}]}' | \
  jq -e '(.ticket | length) > 0'
curl -sf $ADDR/nodes -d '{"ticket": ["kythe:?lang=java#pkg.Names"]}' | \
  jq -e '(.node | length) > 0'
curl -sf $ADDR/edges -d '{"ticket": ["kythe:?lang=java#pkg.Names"]}' | \
  jq -e '(.edge_set | length) > 0'
curl -sf $ADDR/decorations -d '{"location": {"ticket": "kythe://kythe?lang=java?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Names.java#7571dfe62c239daa2caaeed97638184533b0526f4ab16c872311c954100d11e3"}, "source_text": true, "references": true}' | \
  jq -e '(.reference | length) > 0
     and (.node | length) == 0
     and (.source_text | type) == "string"
     and (.source_text | length) > 0'
curl -sf $ADDR/decorations -d '{"location": {"ticket": "kythe://kythe?lang=java?path=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Names.java#7571dfe62c239daa2caaeed97638184533b0526f4ab16c872311c954100d11e3"}, "references": true, "filter": ["**"]}' | \
  jq -e '(.reference | length) > 0
     and (.node | length) > 0
     and (.source_text | length) == 0'
