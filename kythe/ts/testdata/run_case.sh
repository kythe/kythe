#!/bin/bash

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

HAD_ERRORS=0
VERIFIER="kythe/cxx/verifier/verifier"
ENTRYSTREAM="kythe/go/platform/tools/entrystream"
INDEXER="kythe/ts/build/lib/main.js"
NODE="node"

if [[ ! -e "${INDEXER}" ]]; then
  echo "The indexer wasn't compiled, so we'll trivially succeed."
  exit 0
fi

# run_case.sh vnames-json -- indexer-argument -- verifier-argument -- input-list
export KYTHE_VNAMES="$1"
VERIFIER_ARGS=()
INDEXER_ARGS=()
INPUT_LIST=()
ARG_STATE=0
shift 1
for arg; do
  if [[ "$arg" == "--" ]]; then
    ARG_STATE=$((ARG_STATE+1))
  else
    case "$ARG_STATE" in
      0) INDEXER_ARGS+=("$arg");;
      1) VERIFIER_ARGS+=("$arg");;
      *) INPUT_LIST+=("$arg");;
    esac
  fi
done
"${NODE}" "${INDEXER}" "${INDEXER_ARGS[@]}" -- "${INPUT_LIST[@]}" \
    | "${ENTRYSTREAM}" --read_json \
    | "${VERIFIER}" "${VERIFIER_ARGS[@]}" "${INPUT_LIST[@]}"
RESULTS=( ${PIPESTATUS[0]} ${PIPESTATUS[1]} ${PIPESTATUS[2]} )
if [ ${RESULTS[0]} -ne 0 ]; then
  echo "[ FAILED INDEX: ${INPUT_LIST[@]} ]"
  HAD_ERRORS=1
elif [ ${RESULTS[1]} -ne 0 ]; then
  echo "[ FAILED JSON PARSE: ${INPUT_LIST[@]} ]"
  HAD_ERRORS=1
elif [ ${RESULTS[2]} -ne 0 ]; then
  echo "[ FAILED VERIFY: ${INPUT_LIST[@]} ]"
  HAD_ERRORS=1
else
  echo "[ OK: $1 ]"
fi
exit "$HAD_ERRORS"
