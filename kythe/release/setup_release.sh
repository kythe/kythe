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

# Marks a new release by incrementing the current version number, modifying both RELEASES.md and
# kythe/release/BUILD, and creating a new local Git commit.

# Dependencies:
#   - https://github.com/clog-tool/clog-cli

# Guide to creating a Github release:
#   1) Run this script on the clean master commit to release
#      This creates a new release branch with a single commit for the new
#      release $VERSION and builds the optimized release archive.
#      $ ./kythe/release/setup_release.sh
#   2) Review and edit updated RELEASES.md log
#   3) Open a Pull Request for review
#   4) "Draft a new release" at https://github.com/kythe/kythe/releases
#   5) Set tag version / release title to the new $VERSION
#   6) Set description to newest section of RELEASES.md
#   7) Upload release outputs (locations printed by the build rule).
#      These files were generated in step 1.
#   8) Mark as "pre-release" and "Save draft"
#   9) Add draft release URL to Pull Request
#   10) Merge Pull Request once it has been accepted
#   11) Edit Github release draft to set the tag's commit as the freshly pushed
#       release commit
#   12) "Publish release"

cd "$(dirname "$0")"/../..

if [[ "$(git rev-parse --abbrev-ref HEAD)" != "master" ]]; then
  echo "ERROR: not on master branch" >&2
  exit 1
elif [[ -n "$(git diff --name-only)" ]]; then
  echo "ERROR: locally modified files" >&2
  git diff --name-only >&2
  exit 1
fi

# Make sure https://github.com/clog-tool/clog-cli is installed.
hash clog || { echo "ERROR: Please install clog-cli"; exit 1; }

if ! clog --setversion VERSION </dev/null 2>/dev/null | grep -q VERSION; then
  echo "ERROR: clog appears to not be functioning" >&2
  echo "ERROR: you may have 'colorized log filter' on your PATH rather than clog-cli" >&2
  echo "ERROR: please check https://github.com/clog-tool/clog-cli for installation instructions" >&2
fi

previous_version=$(awk '/^release_version =/ { print substr($3, 2, length($3)-2) }' kythe/release/BUILD)

echo "Previous version: $previous_version"
if [[ "$previous_version" != v*.*.* ]]; then
  echo "Unexpected version format" >&2
  exit 1
fi
previous_version=${previous_version#v}

# shellcheck disable=SC2206
components=(${previous_version//./ })
((components[2]++))

join() { local IFS="$1"; shift; echo "$*"; }
version="v$(join . "${components[@]}")"
echo "Marking release $version"

# Update RELEASES.md
{
  cat <<EOF
# Release Notes

## [$version] - $(date +%Y-%m-%d)
EOF
  clog --from "v${previous_version}" --setversion "$version" -r https://github.com/kythe/kythe | tail -n+4
  tail -n+2 RELEASES.md | sed \
    -e "s/\[Unreleased\]/[$version]/" \
    -e "s/HEAD/$version/" \
    -e "/^\[$version\]/i \
[Unreleased] https://github.com/kythe/kythe/compare/${version}...HEAD"
} > RELEASES.md.new
mv -f RELEASES.md.new RELEASES.md

# Update release_version stamp for binaries
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
git commit -anm "release: $version"

# Build and test the Kythe release archive.
bazel --bazelrc=/dev/null test --config=release //kythe/release:release_test
bazel --bazelrc=/dev/null build --config=release //kythe/release
