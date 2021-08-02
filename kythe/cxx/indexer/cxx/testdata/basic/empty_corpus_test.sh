#!/bin/bash
set -euo pipefail

# Simple test that pipes a gzipped entrystream into the empty corpus checker to
# verify that all vnames have a non-empty corpus.

ENTRIES="$1"

CORPUS_CHECKER="kythe/go/test/tools/empty_corpus_checker"
ENTRYSTREAM="kythe/go/platform/tools/entrystream/entrystream"

gunzip -c "$ENTRIES" | "$ENTRYSTREAM" | "$CORPUS_CHECKER"
