#!/bin/bash -e

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

# Kythe's arcanist script-and-regex linter. Expected output format:
#   /^((?P<name>.+?)::)?(?P<severity>warning|error|advice):(?P<line>\\d+)? (?P<message>.*)$/m
#
# Usage: linter.sh <file>
#
# Optionally uses shellcheck if it is found on PATH along with jq.
#
# Arcanist Documentation:
#   https://secure.phabricator.com/book/phabricator/article/arcanist_lint_script_and_regex/

readonly file="$1"
readonly name="$(basename "$1")"
readonly dir="$(dirname "$1")"

case $file in
  AUTHORS|CONTRIBUTORS|WORKSPACE|third_party/*|tools/*|*.md|*BUILD|*/testdata/*|*.yaml|*.json|*.html|*.pb.go|.arclint|.gitignore|*/.gitignore|.arcconfig|*/__phutil_*|*.bzl|.kythe|kythe/web/site/*|go.mod|go.sum|*bazelrc|*.yml|.bazel*version)
    ;; # skip copyright checks
  *)
    if ! grep -q 'Copyright 201[4-9] The Kythe Authors. All rights reserved.' "$file"; then
      echo 'copyright header::error:1 File missing copyright header'
    fi ;;
esac

# Ensure consistent code style
case $file in
  */testdata/*)
    ;; # skip style checks over testdata
  BUILD|*.BUILD|*.bzl)
    if command -v buildifier &>/dev/null; then
      buildifier --mode=check "$file" | sed 's/^/buildifier::error:1 /'
    fi ;;
  *.sh|*.bash)
    if command -v shellcheck &>/dev/null && command -v jq &>/dev/null; then
      shellcheck -f json "$file" | \
        jq -r '.[] | "shellcheck::" + (if .level == "info" then "advice" else .level end) + ":" + (.line | tostring) + " " + .message'
    fi ;;
  *.java)
    if command -v google-java-format &>/dev/null; then
      google-java-format -n "$file" | sed 's/^/google-java-format::error:1 /'
    fi ;;
  *.go)
    if command -v gofmt &>/dev/null; then
      gofmt -l "$file" | sed 's/^/gofmt::error:1 /'
    fi ;;
  *.h|*.cc|*.c|*.proto|*.js)
    if command -v clang-format &>/dev/null; then
      if ! diff -q <(clang-format --style=file "$file") "$file"; then
        echo "clang-format::error:1 $file"
      fi
    fi ;;
esac

# Ensure filenames/paths do not clash on case-insensitive file systems.
if grep -q [A-Z] <<<"$dir"; then
  echo "case-insensitivity::error:1 $dir directory contains an uppercase letter"
fi
if [[ $(find "$dir" -maxdepth 0 -iname "$name" | wc -l) -gt 1 ]]; then
  echo "case-insensitivity::error:1 $name filename clashes on case-insensitive file systems"
fi
