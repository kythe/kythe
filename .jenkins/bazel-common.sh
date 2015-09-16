#!/bin/bash -e
#
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

cd "$WORKSPACE/repo"
.jenkins/get-llvm.sh "$WORKSPACE"

readonly IMAGE=gcr.io/kythe_repo/kythe-builder

gcloud docker --server=beta.gcr.io pull beta.$IMAGE
docker tag -f beta.$IMAGE $IMAGE

docker run --rm -t -v "$PWD:/repo" -w /repo \
 $IMAGE ./setup_bazel.sh

bazel() {
 docker run --rm -t \
   -v "$PWD:/repo" -v "$WORKSPACE/cache:/root/.cache" \
   -w /repo \
   --privileged --entrypoint /usr/bin/bazel \
   $IMAGE "$@"
}

BAZEL_ARGS=(
  --color=no
  --noshow_loading_progress
  --noshow_progress
  --verbose_failures
  --test_output=errors
  --test_summary=terse
  --test_tag_filters=-flaky
)
