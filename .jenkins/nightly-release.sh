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

## Jenkins job configuration:
##   Source Code Management:
##     https://github.com/google/kythe.git
##     Clean before checkout
##     Check out to a sub-directory: repo
##   Build triggers:
##     Poll SCM: H H/8 * * *
##   Build:
##     Execute shell: repo/.jenkins/nightly-release.sh
##   Post-build Actions:
##     E-mail Notification: kythe-ci@google.com

. "$(dirname "$0")/bazel-common.sh"

VERSION="nightly-$(date +%F-%H)-$(git rev-parse --short @)"
BAZEL_ARGS+=(-c opt)
BUCKET=kythe-releases

sed -ri "s/^release_version = .+\$/release_version = \"${VERSION}\"/" \
  kythe/release/BUILD

bazel test "${BAZEL_ARGS[@]}" //kythe/release:release_test

GENFILES="$(bazel info "${BAZEL_ARGS[@]}" | \
  awk '/^bazel-genfiles: / { print $2 }' | tr --delete '\r')"
GENFILES="${GENFILES//\/root\/.cache/$WORKSPACE\/cache}"

gsutil cp "$GENFILES"/kythe/release/kythe-$VERSION.tar.gz{,.md5} gs://$BUCKET/
