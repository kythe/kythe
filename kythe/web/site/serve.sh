#!/bin/bash
set -e
JEKYLL_BIN="$(realpath "$JEKYLL_BIN")"
cd "kythe/web/site"
exec "${JEKYLL_BIN}" serve --skip-initial-build --no-watch "$@"
