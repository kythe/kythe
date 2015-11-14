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

META_INDEX="kythe/cxx/tools/meta_index/meta_index"
INDEX_REPO="kythe/ts/testdata/repo"
CJS_CACHE="kythe/ts/testdata/repo/meta_cache/commonjs_registry"
VCS_CACHE="kythe/ts/testdata/repo/meta_cache/vcs.cache"
VERIFIER="kythe/cxx/verifier/verifier"
JQ="third_party/jq/jq"

"${META_INDEX}" --allow_network=false \
    --commonjs_registry="" \
    --commonjs_registry_cache="${CJS_CACHE}" \
    --vcs_cache="${VCS_CACHE}" \
    --default_local_vcs_id="localsha" \
    --shout="actual_script.sh" \
    --repo_subgraph="actual_subgraph.entries" \
    --vnames="actual_vnames.json" \
    "${INDEX_REPO}"

"${VERIFIER}" "kythe/cxx/tools/meta_index/testdata/expected_entries.verifier" \
    < actual_subgraph.entries

"${JQ}" . "actual_vnames.json" \
    | sed 's:\\\\::g' \
    | sed "s:${PWD}::g" \
    | diff - "kythe/cxx/tools/meta_index/testdata/expected_vnames.json"

sed "s:${PWD}::g" actual_script.sh \
    | diff - "kythe/cxx/tools/meta_index/testdata/expected_script.sh"
