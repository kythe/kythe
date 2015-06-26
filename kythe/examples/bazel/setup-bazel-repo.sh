#!/bin/bash -e
#
# Copyright 2015 Google Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Utility script to setup the Bazel repository to work with the kythe-index.sh script.

EXAMPLE_ROOT="$(realpath -s "$(dirname "$0")")"

usage() {
  cat <<EOF
Usage: setup-bazel-repo.sh <bazel-root> [kythe-release-root]

Utility script to setup the Bazel repository to work with the kythe-index.sh script.  The assumed
Kythe release root is /opt/kythe if unspecified.
EOF
}

BAZEL_ROOT=
KYTHE_RELEASE=/opt/kythe

case $# in
  1)
    BAZEL_ROOT="$1" ;;
  2)
    BAZEL_ROOT="$1"
    KYTHE_RELEASE="$(realpath -s $2)" ;;
  *)
    usage
    exit 1 ;;
esac

cd "$BAZEL_ROOT"

if [[ -d third_party/kythe ]]; then
  echo "ERROR: $BAZEL_ROOT looks setup already" >&2
  echo "Maybe run \`git clean -fd\` in $BAZEL_ROOT?" >&2
  exit 1
fi

if [[ ! -d "$KYTHE_RELEASE/indexers" ]]; then
  echo "ERROR: $KYTHE_RELEASE doesn't look like a Kythe release" >&2
  exit 1
elif [[ ! -x "$KYTHE_RELEASE/extractors/bazel_cxx_extractor" ]]; then
  echo "ERROR: the Kythe release at $KYTHE_RELEASE is too old; please update it" >&2
  exit 1
fi

mkdir third_party/kythe
cp "$KYTHE_RELEASE/LICENSE" "$KYTHE_RELEASE"/extractors/bazel_* third_party/kythe/
cp "$EXAMPLE_ROOT/BUILD.third_party" third_party/kythe/BUILD

mkdir src/data
cp "$EXAMPLE_ROOT/BUILD.src.data" src/data/BUILD
cp "$EXAMPLE_ROOT/kythe_config.json" src/data/
