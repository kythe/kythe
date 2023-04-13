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

# A pre-commit hook which will determine and run affected test targets using the
# files passed on the command line.
# Usage: test-affected-targets.sh file...
set -e
OMIT_TAGS=(manual broken arc-ignore docker)

function join_by {
  local d="$1"
  shift
  local f="$1"
  shift
  printf %s "$f" "${@/#/$d}";
}

TAG_RE="\\b($(join_by '|' "${OMIT_TAGS[@]}"))\\b"

readarray -t TARGETS < <(bazel query \
  --keep_going \
  --noshow_progress \
  "let exclude = attr('tags', '$TAG_RE', //...) in rdeps(//... except \$exclude, set($*)) except \$exclude")

if [[ "${#TARGETS[@]}" -gt 0 ]]; then
  echo "Building targets"
  bazel build --config=prepush "${TARGETS[@]}"
fi

readarray -t TESTS < <(bazel query \
  --keep_going \
  --noshow_progress \
  "let expr = tests(set(${TARGETS[*]})) in \$expr except attr('tags', '$TAG_RE', \$expr)")

if [[ "${#TESTS[@]}" -gt 0 ]]; then
  echo "Running tests"
  bazel test --config=prepush "${TESTS[@]}"
fi
