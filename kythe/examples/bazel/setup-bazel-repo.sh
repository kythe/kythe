#!/bin/bash -e
#
# Copyright 2015 The Kythe Authors. All rights reserved.
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
Usage: setup-bazel-repo.sh <bazel_root>

Utility script to setup the Bazel repository to work with the kythe-index.sh script.
EOF
}

BAZEL_ROOT=

case $# in
  1)
    BAZEL_ROOT="$1" ;;
  *)
    usage
    exit 1 ;;
esac

cd "$BAZEL_ROOT"

if grep -q 'Kythe extraction setup' WORKSPACE; then
  echo "ERROR: $BAZEL_ROOT looks setup already" >&2
  echo "Maybe run \`git checkout . && git clean -fd\` in $BAZEL_ROOT?" >&2
  exit 1
fi

# TODO(#3272): Kythe's protobuf dependency conflicts with Bazel's vendoring
sed -i 's/"com_google_protobuf"/"com_google_protobuf_vendored"/' WORKSPACE

cat "${EXAMPLE_ROOT}/bazel.WORKSPACE" >> WORKSPACE
