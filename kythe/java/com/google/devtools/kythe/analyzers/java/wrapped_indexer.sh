#!/bin/bash
#
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
RUNFILES=${RUNFILES:-$0.runfiles/io_kythe}
RUNFILES=$("$RUNFILES/tools/modules/abspath" "$RUNFILES")

_ARGS=()
for path in "${@:1:${#@}}"; do
  case "$path" in
    -*)
      _ARGS+=("$path") ;;
    *)
      if [[ ! -r "$path" ]]; then
        if [[ -r "$RUNFILES/${path##bazel-out/host/bin/}" ]]; then
          path="$RUNFILES/${path##bazel-out/host/bin/}"
        fi
      fi
      _ARGS+=("$("${RUNFILES}/tools/modules/abspath" "${path}")") ;;
  esac
done

exec "${RUNFILES}/tools/cdexec/cdexec" \
  -t java_indexer.XXXXXX \
  "${RUNFILES}/kythe/java/com/google/devtools/kythe/analyzers/java/indexer" \
  "${_ARGS[@]}"
