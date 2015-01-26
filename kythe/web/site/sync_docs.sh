#!/bin/bash -e
#
# Copyright 2014 Google Inc. All rights reserved.
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
#
# Script to sync Kythe documentation into /_docs/ for kythe.io.
#
# Usage: ./sync_docs.sh

export SHELL=/bin/bash

DIR="$(readlink -e "$(dirname "$0")")"
cd "$DIR/../../.."

export DOCS_DIR=kythe/docs
./campfire build --asciidoc_backend=html --asciidoc_partial "//$DOCS_DIR/..."
rsync -a --delete "campfire-out/doc/$DOCS_DIR/" "$DIR"/_docs
DOCS=($(./campfire query --print_names "files('//$DOCS_DIR/...')" | \
  grep -E '\.(txt|adoc|ad)$' | \
  parallel 'x() { echo "${1#$DOCS_DIR/}"; }; x'))

asciidoc_query() {
  bundle exec ruby -r asciidoctor -e 'puts Asciidoctor.load_file(ARGV[0]).'"$2" "$1" 2>/dev/null
}

asciidoc_attribute_presence() {
  [[ "$(asciidoc_query "$1" "attributes[\"$2\"] != nil")" == "true" ]]
}

doc_header() {
  echo "---
layout: page
title: $(asciidoc_query "$1" doctitle)
priority: $(asciidoc_query "$1" 'attributes["priority"]')
toclevels: $(asciidoc_query "$1" 'attributes["toclevels"]')"
  if asciidoc_attribute_presence "$1" toc || asciidoc_attribute_presence "$1" toc2; then
    echo "toc: true"
  fi
echo "---"
}

TMP="$(mktemp)"
trap 'rm -rf "$TMP"' EXIT ERR INT

cd "$DIR"
for doc in ${DOCS[@]}; do
  html=${doc%%.*}.html
  abs_path="../../../$DOCS_DIR/$doc"
  cp "_docs/$html" "$TMP"
  { doc_header "$abs_path";
    cat "$TMP"; } > "_docs/$html"
done

mv _docs/schema/{schema,index}.html
