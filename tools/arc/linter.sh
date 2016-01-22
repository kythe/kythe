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
readonly dir="$(dirname "$1")"

case $file in
  WORKSPACE|third_party/*|tools/*|*.md|BUILD|*/BUILD|*/testdata/*|*.yaml|*.json|*.html|*.pb.go|.arclint|.gitignore|*/.gitignore|.arcconfig|*/__phutil_*|*.bzl|.kythe|kythe/web/site/*)
    ;; # skip copyright checks
  *)
    if ! grep -q 'Copyright 201[4-9] Google Inc. All rights reserved.' "$file"; then
      echo 'copyright header::error: File missing copyright header'
    fi ;;
esac

# Ensure filenames/paths do not clash on case-insensitive file systems.
if grep -q [A-Z] <<<"$dir"; then
  echo "case-insensitivity::error: $dir directory contains an uppercase letter"
fi
if [[ $(find "$dir" -maxdepth 1 -iname "$name" | wc -l) -gt 1 ]]; then
  echo "case-insensitivity::error: $name filename clashes on case-insensitive file systems"
fi
