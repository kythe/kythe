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
##     Build periodically: H H(10-14) * * *
##     Poll SCM: H/5 * * * *
##   Build:
##     Execute shell: repo/.jenkins/bazel-build-test.sh
##   Post-build Actions:
##     E-mail Notification: kythe-ci@google.com

. "$(dirname "$0")/bazel-common.sh"
bazel test "${BAZEL_ARGS[@]}" //kythe/docs/schema //...
