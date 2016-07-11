#!/bin/bash -e
set -o pipefail
# Copyright 2016 Google Inc. All rights reserved.
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
# This script tests that the java indexer doesn't fall over on empty CUs.

indexer="kythe/java/com/google/devtools/kythe/analyzers/java/indexer"
entrystream="kythe/go/platform/tools/entrystream/entrystream"
test_kindex="$PWD/kythe/testdata/java_empty.kindex"

# Test indexing a .kindex file
$indexer "$test_kindex" | $entrystream >/dev/null
