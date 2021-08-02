#!/bin/bash
# Copyright 2021 The Kythe Authors. All rights reserved.
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

set -euo pipefail

# Simple test that pipes a gzipped entrystream into the empty corpus checker to
# verify that all vnames have a non-empty corpus.

ENTRIES="$1"

CORPUS_CHECKER="kythe/go/test/tools/empty_corpus_checker"
ENTRYSTREAM="kythe/go/platform/tools/entrystream/entrystream"

gunzip -c "$ENTRIES" | "$ENTRYSTREAM" | "$CORPUS_CHECKER"
