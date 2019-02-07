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

# TODO(danielmoy): for modifying other bazel repos, use the setup-bazel-repo script.
ADD kythe/examples/bazel/setup-bazel-repo.sh /opt/kythe/extractors/setup-bazel-repo.sh

ENV KYTHE_DEPS \
    asciidoc asciidoctor source-highlight graphviz git \
    gcc libssl-dev uuid-dev libncurses-dev libcurl4-openssl-dev flex clang-3.6 bison cmake \
    openjdk-8-jdk \
    parallel wget unzip curl ca-certificates \
    pkg-config zip g++ zlib1g-dev python

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends $KYTHE_DEPS && \
    apt-get clean

# Actually still more deps, because of bazel.
RUN wget -O /tmp/bazel-installer.sh https://github.com/bazelbuild/bazel/releases/download/0.19.2/bazel-0.19.2-installer-linux-x86_64.sh
RUN chmod +x /tmp/bazel-installer.sh
RUN /tmp/bazel-installer.sh

# Get the kythe repo in /opt/kythe/repo
ENV KYTHE_VERSION=434c0b0f25ee645e7b95c13adf497a80dc72f754
RUN mkdir -p /opt/kythe/repo
RUN mkdir /tmp/kythe
RUN wget -O /tmp/kythe/kythe.zip https://github.com/kythe/kythe/archive/$KYTHE_VERSION.zip
RUN unzip /tmp/kythe/kythe.zip -d /tmp/kythe
RUN mv -T /tmp/kythe/kythe-$KYTHE_VERSION /opt/kythe/repo/kythe

# Set up the kythe repo
WORKDIR /opt/kythe/repo/kythe
ENV CC=/usr/bin/clang-3.6
