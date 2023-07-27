# Copyright 2021 The Kythe Authors. All rights reserved.
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
# A minimal image for building Kythe with Bazelisk

# docker build -t gcr.io/kythe-repo/bazelisk-builder-client .
FROM ubuntu:23.04

RUN apt-get update \
        && apt-get upgrade -y \
        && apt-get install -y --no-install-recommends \
                curl ca-certificates \
                # Bazel's fallback @local_jdk and needed by rules_jvm_external
                openjdk-11-jdk-headless \
                # Used to build the website; rules_ruby doesn't support alternate toolchains well.
                ruby \
                # Needed for io_bazel_rules_docker which is used by bazel_toolchains.
                git \
                # Needed to apply local patches to Bazel reposiories.
                patch \
                # Needed to configure the LLVM repository.
                python3 \
        && apt-get clean \
        && rm -rf /var/lib/apt/lists/*


# Fetch the latest version of Bazelisk for AMD64 Linux from GitHub
RUN curl -s https://api.github.com/repos/bazelbuild/bazelisk/releases/latest \
        | sed -n '/browser_download_url/s/[^:]*:[^"]*\("[^"]*"\).*/url = \1/p' \
        | grep bazelisk-linux-amd64 \
        | curl -L -o /usr/bin/bazelisk -K - \
        && chmod +x /usr/bin/bazelisk

# Add the wrapper script for USE_BAZEL_VERSION=min support.
ADD bazel /usr/bin/bazel
# Use a custom system bazelrc file as well.
ADD bazelrc /etc/bazel.bazelrc
