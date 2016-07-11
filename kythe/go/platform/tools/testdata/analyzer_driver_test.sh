#!/bin/bash -e

analyzer_driver() { "$PWD/kythe/go/platform/tools/analyzer_driver/analyzer_driver" "$@"; }
entrystream() { "$PWD/kythe/go/platform/tools/entrystream/entrystream" "$@"; }

JAVA_INDEXER_SERVER="$PWD/kythe/java/com/google/devtools/kythe/analyzers/java/indexer_server"
KINDEX="$PWD/kythe/testdata/test.kindex"

COUNT=$(analyzer_driver -- "$JAVA_INDEXER_SERVER" --port=@port@ -- "$KINDEX" | \
  entrystream --count)
test "$COUNT" -ne 0

COUNT=$(analyzer_driver "$JAVA_INDEXER_SERVER" --port=@port@ -- "$KINDEX"{,} | \
  entrystream --count)
test "$COUNT" -ne 0
