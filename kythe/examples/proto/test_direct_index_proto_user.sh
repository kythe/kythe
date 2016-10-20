#!/bin/bash -e
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
# usage: test_direct_index_proto_user.sh [artifact-dir]
# If artifact-dir is not provided, a temporary directory will be used.

set -o pipefail
set -o errexit

cd "$(bazel info workspace)"

if [[ -d "$1" ]]; then
  D="$1"
else
  D="$(mktemp -d 2>/dev/null || mktemp -d -t 'kythetest')"
  trap 'rm -rf "${D}"' EXIT ERR INT
fi

# Generate the proto descriptor.
bazel build //kythe/examples/proto:example.descriptor

# Index the proto descriptor.
BAZELGENFILES="$(bazel info workspace)/bazel-genfiles"
bazel build //kythe/examples/proto:proto_indexer \
            //kythe/cxx/verifier:verifier \
            //kythe/cxx/indexer/cxx:indexer
./bazel-bin/kythe/examples/proto/proto_indexer \
    "${BAZELGENFILES}"/kythe/examples/proto/example.descriptor \
        > "${D}/proto.entries"

# Verify that part of the index.
./bazel-bin/kythe/cxx/verifier/verifier kythe/examples/proto/example.proto \
    < "${D}/proto.entries"

# Generate the kindex for the proto-using CU.
# This will include the .meta file (because of the kythe_metadata pragma).
BAZELOUT="$(bazel info workspace)/bazel-out"

# Remove any old kindexes.
pushd "$BAZELOUT"
find . \
  -path "*/extra_actions/kythe/cxx/extractor/extra_action/kythe/examples/proto/*.kindex" \
  -delete
popd

bazel build \
  --experimental_action_listener=//kythe/cxx/extractor:extract_kindex \
  --experimental_extra_action_top_level_only \
  --experimental_proto_extra_actions \
  //kythe/examples/proto:proto_user

pushd "$BAZELOUT"
find . \
  -path "*/extra_actions/kythe/cxx/extractor/extra_action/kythe/examples/proto/*.kindex" \
  -exec cp {} "${D}/proto_user.kindex" \; \
  && \
  chmod 644 "${D}/proto_user.kindex"
popd

echo "Running indexer and verifier using ${D}/proto_user.kindex"
./bazel-bin/kythe/cxx/indexer/cxx/indexer "${D}/proto_user.kindex" \
    > "${D}/proto_user.entries"
cat "${D}/proto_user.entries" "${D}/proto.entries" |
    ./bazel-bin/kythe/cxx/verifier/verifier --ignore_dups \
        kythe/examples/proto/example.proto \
        kythe/examples/proto/proto_user.cc
