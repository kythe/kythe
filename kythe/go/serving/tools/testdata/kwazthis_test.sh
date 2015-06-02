#!/bin/bash -e
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
set -o pipefail

BASE_DIR="$TEST_SRCDIR/kythe/go/serving/tools/testdata"
OUT_DIR="$TEST_TMPDIR"

TEST_ENTRIES="$TEST_SRCDIR/kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/generics_tests.entries"
source "kythe/cxx/common/testdata/start_http_service.sh"

jq () { "third_party/jq/jq" -e "$@" <<<"$JSON"; }
kwazthis() { "kythe/go/serving/tools/kwazthis" --ignore_local_repo --api "http://$LISTEN_AT" "$@"; }

PATH=kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/Generics.java

JSON=$(kwazthis --path $PATH --offset 783)
jq --slurp 'length == 2'
jq --slurp '.[0].span.text == "Generics<String>"'
jq --slurp '.[1].span.text == "String"'
jq --slurp '.[].kind == "ref"'
jq --slurp '.[].node.ticket
        and .[].node.ticket != ""'
jq --slurp '.[].node.kind
        and .[].node.kind != ""'

JSON=$(kwazthis --path $PATH --offset 558)
jq --slurp 'length == 1'
jq '.kind == "defines"'
jq '.span.text == "g"'
jq '.span.start == 558'
jq '.span.end == 559'
jq '.node.ticket'
jq '.node.ticket != ""'
jq '.node.kind == "function"'
jq '(.node.names | length) == 1'
