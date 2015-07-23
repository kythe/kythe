#!/bin/bash -e

analyzer_driver() { "$TEST_SRCDIR"/kythe/go/platform/tools/analyzer_driver "$@"; }
entrystream() { "$TEST_SRCDIR"/kythe/go/platform/tools/entrystream "$@"; }

JAVA_INDEXER_SERVER="$TEST_SRCDIR"/kythe/java/com/google/devtools/kythe/analyzers/java/indexer_server
KINDEX="$TEST_SRCDIR/kythe/testdata/test.kindex"

COUNT=$(analyzer_driver -- "$JAVA_INDEXER_SERVER" --port=@port@ -- "$KINDEX" | \
  entrystream --count)
test "$COUNT" -eq 158

COUNT=$(analyzer_driver "$JAVA_INDEXER_SERVER" --port=@port@ -- "$KINDEX"{,} | \
  entrystream --count)
test "$COUNT" -eq 316
