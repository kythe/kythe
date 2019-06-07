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

FROM debian:stretch

RUN echo "deb http://ftp.debian.org/debian stretch-backports main" >>/etc/apt/sources.list

RUN apt-get update && \
    apt-get upgrade -y && \
    # Required by go tool build of Kythe
    apt-get -t stretch-backports install -y libbrotli-dev && \
    apt-get install -y \
      # Buildbot dependencies
      python3 python3-dev python3-pip wget git \
      # Required by go tool build of Kythe
      libleveldb-dev \
      # Bazel's fallback @local_jdk
      openjdk-8-jdk \
      # Bazel dependencies
      pkg-config zip g++ zlib1g-dev unzip \
      # Kythe C++ dependencies
      gcc uuid-dev flex clang-3.8 bison \
      # Kythe misc dependencies
      asciidoc source-highlight graphviz curl parallel && \
    apt-get clean

# Install Buildbot
RUN pip3 install --upgrade pip
RUN pip3 install buildbot
RUN pip3 install buildbot-www buildbot-console-view buildbot-grid-view buildbot-waterfall-view psycopg2-binary txrequests
RUN pip3 install --upgrade six service_identity pyasn1 cryptography pyopenssl
RUN pip3 install buildbot-worker

# Kythe symlink for Kythe to pickup clang installation
RUN ln -s /usr/bin/clang-3.8 /usr/bin/clang && \
    ln -s /usr/bin/clang++-3.8 /usr/bin/clang++

# Install Go
RUN wget https://dl.google.com/go/go1.12.5.linux-amd64.tar.gz && \
    tar -C /usr/local -xzf go*.tar.gz && \
    rm -rf go*.tar.gz
ENV PATH=$PATH:/usr/local/go/bin

# Install Bazelisk and wrapper script
ADD bazel /usr/bin/bazel
RUN curl -L -o /usr/bin/bazelisk https://github.com/bazelbuild/bazelisk/releases/download/v0.0.7/bazelisk-linux-amd64 && chmod +x /usr/bin/bazelisk

# Buildbot configuration
ADD bazelrc /root/.bazelrc
ADD start.sh /buildbot/
ADD worker /buildbot/worker
ADD master /buildbot/master
ADD secrets.tar /buildbot
RUN buildbot checkconfig /buildbot/master/master.cfg

EXPOSE 8080
CMD /buildbot/start.sh
