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

# Marks a new release by incrementing the current version number, modifying both RELEASES.md and
# kythe/release/BUILD, and creating a new local Git commit.

cd "$(dirname $0)"/../..

if [[ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]]; then
  echo "ERROR: not on master branch" >&2
  exit 1
elif [[ -n "$(git diff --name-only)" ]]; then
  echo "ERROR: locally modified files" >&2
  git diff --name-only >&2
  exit 1
fi

previous_version=$(awk '/^release_version =/ { print substr($3, 2, length($3)-2) }' kythe/release/BUILD)

echo "Previous version: $previous_version"
if [[ "$previous_version" != v*.*.* ]]; then
  echo "Unexpected version format" >&2
  exit 1
fi
previous_version=${previous_version#v}

components=(${previous_version//./ })
((components[2]++))

join() { local IFS="$1"; shift; echo "$*"; }
version="v$(join . "${components[@]}")"
echo "Marking release $version"

sed -i "s/Upcoming release/$version/" RELEASES.md
sed -ri "s/^release_version = .+/release_version = \"$version\"/" kythe/release/BUILD

if ! diff -q <(git diff --name-only) <(echo RELEASES.md; echo kythe/release/BUILD) >/dev/null; then
  if [[ -z "$(git diff --name-only -- RELEASES.md)" ]]; then
    echo "ERROR: RELEASES.md has not been updated (no listed changes?)" >&2
    exit 1
  elif [[ -z "$(git diff --name-only -- kythe/release/BUILD)" ]]; then
    echo "ERROR: kythe/release/BUILD has not been updated" >&2
    exit 1
  fi
  echo "Unexpected changed files in repository:" >&2
  git diff --name-only | grep -v -e '^RELEASES.md$' -e '^kythe/release/BUILD$'
fi

git checkout -b "release-$version"
git commit -am "Setup release $version"
