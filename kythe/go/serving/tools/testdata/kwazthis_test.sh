#!/bin/bash
# Copyright 2015 The Kythe Authors. All rights reserved.
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
set -eox pipefail

BASE_DIR="$PWD/kythe/go/serving/tools/testdata"
OUT_DIR="$TEST_TMPDIR"

TEST_ENTRIES="$PWD/kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/generics_tests_entries.entries.gz"
source "kythe/cxx/common/testdata/start_http_service.sh"

jq () { "$JQ" -e "$@" <<<"$JSON"; }
kwazthis() { "$KWAZTHIS" --local_repo=NONE --api "http://$LISTEN_AT" "$@" | tee /dev/stderr; }

FILE_PATH=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Generics.java

JSON=$(kwazthis --corpus kythe --path $FILE_PATH --offset 813)
jq --slurp 'length == 4'
# .[0] is Generics class def
# .[1] is f method def
# .[2] is gs variable def
# .[3] is gs variable defines/binding
jq --slurp '.[] | (.kind == "ref" or .kind == "defines" or .kind == "defines/binding")'
jq --slurp '.[].node.ticket
        and .[].node.ticket != ""'
jq --slurp '.[].node.kind
        and .[].node.kind != ""'
