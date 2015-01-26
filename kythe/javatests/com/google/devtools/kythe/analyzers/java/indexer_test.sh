#!/bin/bash -e
set -o pipefail
#
# This script tests the java indexer's CLI.

indexer=campfire-out/bin/kythe/java/com/google/devtools/kythe/analyzers/java/indexer
indexpack=campfire-out/bin/kythe/go/platform/tools/indexpack
entrystream=campfire-out/bin/kythe/go/platform/tools/entrystream
test_kindex=kythe/testdata/test.kindex

# Test indexing a .kindex file
$indexer $test_kindex | $entrystream >/dev/null

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT ERR INT

UNIT="$($indexpack --to_archive "$tmp/archive" $test_kindex 2>/dev/null)"

# Test indexing compilation in an indexpack
$indexer --index_pack="$tmp/archive" $UNIT | $entrystream >/dev/null
