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

# Guide to creating a Github release:
#   1) Run this script on the clean master commit to release
#      This creates a new release branch with a single commit for the new
#      release $VERSION
#      $ ./kythe/release/setup_release.sh
#   2) Send the commit to Phabricator for review
#      $ arc diff
#   3) Build/test Kythe release archive w/ optimizations:
#      $ bazel test -c opt //kythe/release:release_test
#   4) "Draft a new release" at https://github.com/google/kythe/releases
#   5) Set tag version / release title to the new $VERSION
#   6) Set description to newest section of RELEASES.md
#   7) Upload bazel-genfiles/kythe/release/kythe-$VERSION.tar.gz{,.md5}
#      These files were generated in step 3.
#   8) Mark as "pre-release" and "Save draft"
#   9) Send draft release URL to Phabricator review
#   10) Push release commit once Phabricator review has been accepted
#   11) Edit Github release draft to set the tag's commit as the freshly pushed
#       release commit
#   12) "Publish release"

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
