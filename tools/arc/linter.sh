#!/bin/sh -e

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

# Kythe's arcanist script-and-regex linter. Expected output format:
#   /^((?P<name>.+?)::)?(?P<severity>warning|error|advice):(?P<line>\\d+)? (?P<message>.*)$/m
#
# Usage: linter.sh <file>
#
# Arcanist Documentation:
#   https://secure.phabricator.com/book/phabricator/article/arcanist_lint_script_and_regex/

readonly file="$1"
readonly name="$(basename "$1")"

lint_copyright() {
  case $file in
    third_party/*|*.md|BUILD|*/BUILD|buildtools/*|*/testdata/*|*.yaml|*.json|*.html|*.pb.go|.arclint|.gitignore|*/.gitignore|.arcconfig|*/__phutil_*|*.bzl|.kythe|kythe/web/site/*)
      ;; # skip copyright checks
    *)
      if ! grep -Pq 'Copyright 201[45] Google Inc. All rights reserved.' "$file"; then
        echo 'copyright header::error: File missing copyright header'
      fi ;;
  esac
}

case "$name" in
  *) lint_copyright;;
esac
