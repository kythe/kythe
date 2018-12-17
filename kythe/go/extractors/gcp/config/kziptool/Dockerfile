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

# Build: bazel build //kythe/go/extractors/gcp/bazel:artifacts
# Usage:
#   This container houses bazel extraction artifacts which are necessary for
#   extracting a bazel repo.  It includes a script for modifying a repo's
#   WORKSPACE in /opt/kythe/extractors/setup-bazel-repo.sh, as well as an
#   installed copy of the kythe repo at /opt/kythe/repo/kythe.
#
#   From commandline:
#     docker run -it --entrypoint "/bin/sh" gcr.io/kythe-public/kythe-bazel-extractor-artifacts
#
#   From Google Cloud Build:
#     - name: 'gcr.io/kythe-public/kythe-bazel-extractor-artifacts'
#       volumes:
#         - name: 'kythe_extractors'
#           path: '/opt/kythe/extractors/'
#         - name: 'kythe_repo'
#           path: '/opt/kythe/repo/kythe/'

FROM launcher.gcr.io/google/ubuntu16_04

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends zipmerge && \
    apt-get clean

# TODO(danielmoy): when we include other tools, don't use an entypoint like this
ENTRYPOINT ["zipmerge"]
