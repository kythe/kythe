# Copyright 2020 The Kythe Authors. All rights reserved.
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

FROM debian:stable-slim

# Install C++ compilers and other deps
# note that openjdk-11-jdk-headless is a dependency of the java extractor
RUN apt-get update -y && \
    # This is temporary until Debian starts shipping merged /usr directories in their images
    apt-mark hold usrmerge usr-is-merged && \
    apt-get upgrade -y && \
    apt-get install -y git clang-15 build-essential zip python3 openjdk-17-jdk-headless && \
    apt-get clean

# Create clang symlinks
RUN ln -s /usr/bin/clang-15 /usr/bin/clang && \
    ln -s /usr/bin/clang++-15 /usr/bin/clang++

RUN echo 'build --client_env=CC=/usr/bin/clang' >> ~/.bazelrc
RUN echo 'build --client_env=CXX=/usr/bin/clang++' >> ~/.bazelrc

# Extract the Kythe release archive to /kythe
COPY kythe/release/kythe-v*.tar.gz /tmp/
RUN tar --no-same-owner -xzf /tmp/kythe-v*.tar.gz && \
    mv kythe-v*/ /kythe && \
    rm /tmp/kythe-v*.tar.gz

# Tools and configuration
ADD kythe/extractors/bazel/extract.sh /kythe/
ADD kythe/extractors/bazel/bazel_wrapper.sh /kythe/
ADD kythe/release/base/fix_permissions.sh /kythe/

# Bazelisk
ADD external/com_github_bazelbuild_bazelisk/bazelisk_/bazelisk /kythe/

RUN mkdir -p /workspace
WORKDIR /workspace

# copied from gcr.io bazel image
# Store the Bazel outputs under /workspace so that the symlinks under bazel-bin
# (et al) are accessible to downstream build steps.
RUN echo 'startup --output_base=/workspace/.bazel' >> ~/.bazelrc

ENTRYPOINT ["/kythe/extract.sh"]
