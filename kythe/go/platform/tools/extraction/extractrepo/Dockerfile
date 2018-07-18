# Copyright 2018 The Kythe Authors. All rights reserved.
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
# $ docker run -i -v /some/mvn/repo:/repo -v /some/output/dir:/output -v /var/run/docker.sock:/var/run/docker.sock -t kythe-mvn-extract-0.2
#
# It will deposit .kindex files into /tmp/output.

FROM docker:dind

# TODO(#156): These volumes are only necessary for docker-in-docker, remove
# them in the future.
VOLUME /tmp/input
VOLUME /tmp/out

ADD extractrepo extractrepo

ENTRYPOINT ["/extractrepo", "-local", "/repo", "-output", "/output", "-tmp_repo_dir", "/tmp/input", "-tmp_out_dir", "/tmp/out"]
