# Copyright 2018 Google Inc. All rights reserved.
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

# Dockerfile for packaging up extractrepo.
#
# You must first build extractrepo using bazel before trying to build this
# image:
#
# $ bazel build kythe/go/platforms/tools/extraction/extractrepo:extractrepo
# $ cp bazel-bin/kythe/go/platform/tools/extraction/extractrepo/linux_amd64_stripped/extractrepo kythe/go/platform/tools/extraction/extractrepo
# $ docker build --tag test-extract kythe/go/platform/tools/extraction/extractrepo/.

FROM docker:dind

VOLUME /tmp/input
VOLUME /tmp/output

ADD extractrepo extractrepo

ENTRYPOINT ["/extractrepo", "-local", "/tmp/repo", "-output", "/tmp/output", "-tmp_repo_dir", "/tmp/input"]
