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

FROM debian:jessie

RUN echo "deb http://ftp.debian.org/debian stretch-backports main" >>/etc/apt/sources.list

RUN apt-get update && \
    apt-get upgrade -y && \
    # Required by go tool build of Kythe
    apt-get -t stretch-backports install -y libbrotli-dev && \
    apt-get install -y \
      # Buildbot dependencies
      python python-dev python-pip wget git \
      # Required by go tool build of Kythe
      libleveldb-dev \
      # Bazel dependencies
      pkg-config zip g++ zlib1g-dev unzip \
      # Kythe C++ dependencies
      gcc libssl-dev uuid-dev libncurses-dev libcurl4-openssl-dev flex clang-3.5 bison \
      # Kythe misc dependencies
      asciidoc source-highlight graphviz curl parallel && \
    apt-get clean

# Install Buildbot
RUN pip install --upgrade pip
RUN pip install buildbot
RUN pip install buildbot-www buildbot-console-view buildbot-grid-view buildbot-waterfall-view psycopg2-binary txrequests
RUN pip install --upgrade six service_identity pyasn1 cryptography pyopenssl
RUN pip install buildbot-worker

# Kythe symlink for Kythe to pickup clang installation
RUN ln -s /usr/bin/clang-3.5 /usr/bin/clang && \
    ln -s /usr/bin/clang++-3.5 /usr/bin/clang++

# We require a newer cmake than in Debian Jessie
RUN wget https://cmake.org/files/v3.11/cmake-3.11.3-Linux-x86_64.sh && \
    sh cmake*.sh --skip-license && \
    rm cmake*.sh

# Install Go
RUN wget https://dl.google.com/go/go1.10.3.linux-amd64.tar.gz && \
    tar -C /usr/local -xzf go*.tar.gz && \
    rm -rf go*.tar.gz
ENV PATH=$PATH:/usr/local/go/bin

# Install Bazel min/max versions
RUN curl -L -o /usr/bin/bazel-0.14.0 https://github.com/bazelbuild/bazel/releases/download/0.14.0/bazel-0.14.0-linux-x86_64 && chmod +x /usr/bin/bazel-0.14.0
RUN curl -L -o /usr/bin/bazel-0.15.2 https://github.com/bazelbuild/bazel/releases/download/0.15.2/bazel-0.15.2-linux-x86_64 && chmod +x /usr/bin/bazel-0.15.2
ADD bazel /usr/bin/bazel

ADD cache-llvm.sh /usr/bin/
RUN chmod +x /usr/bin/cache-llvm.sh && cache-llvm.sh --init

# Buildbot configuration
ADD bazelrc /root/.bazelrc
ADD start.sh /buildbot/
ADD worker /buildbot/worker
ADD master /buildbot/master
ADD secrets/kythe-bazel-remote-cache.json /buildbot/

EXPOSE 8080
CMD /buildbot/start.sh
