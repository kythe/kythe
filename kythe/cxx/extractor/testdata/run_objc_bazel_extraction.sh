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

set -o pipefail

if [ "$(uname)" != "Darwin" ]; then
  echo "Objective C Bazel extraction can only be done on darwin"
  exit 2
fi

BAZELOUT="$(bazel info workspace)/bazel-out"
bazel build \
  --experimental_action_listener=//kythe/cxx/extractor:extract_kindex_objc \
  --experimental_extra_action_top_level_only \
  --experimental_proto_extra_actions \
  --ios_sdk_version=10.0 \
  :objc_lib

if [ $? -ne 0 ]; then
  echo "Build failed"
  exit 1
fi

# HACK: It is possible that there are other xa files in here.
D="$(pwd)"
pushd "$BAZELOUT"
find . \
  -path "*/extra_actions/kythe/cxx/extractor/extra_action_objc/kythe/cxx/extractor/testdata/*.xa" \
  -exec cp {} "$D/objc_lib.xa" \; \
  && \
  chmod -x "$D/objc_lib.xa"
popd
