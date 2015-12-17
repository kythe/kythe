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

# docker build -t gcr.io/kythe_repo/kythe-builder .
FROM debian:jessie

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends curl ca-certificates && \
    apt-get clean

RUN echo deb http://http.debian.net/debian jessie-backports main >> /etc/apt/sources.list

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      git pkg-config zip unzip rsync patch zsh wget net-tools less parallel locales make \
      g++ gcc openjdk-8-jdk openjdk-8-source clang-3.5 flex asciidoc source-highlight graphviz \
      zlib1g-dev libarchive-dev libncurses-dev uuid-dev libssl-dev \
      ca-certificates-java libcurl4-openssl-dev libmemcached-dev libsasl2-dev && \
    apt-get clean

ENV JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
RUN ln -Ts /usr/bin/clang-3.5 /usr/bin/clang

# Bison
RUN wget http://archive.kernel.org/debian-archive/debian/pool/main/b/bison/bison_2.3.dfsg-5_amd64.deb -O /tmp/bison_2.3.deb && \
    dpkg -i /tmp/bison_2.3.deb && \
    rm -f /tmp/bison_2.3.deb

# Go
RUN wget https://storage.googleapis.com/golang/go1.4.linux-amd64.tar.gz -O /tmp/go.tar.gz && \
    tar xzf /tmp/go.tar.gz -C /usr/local/ && \
    rm -f /tmp/go.tar.gz
ENV PATH=/usr/local/go/bin:$PATH

RUN cd /opt && \
    git clone https://github.com/google/bazel/ && \
    cd bazel && \
    git checkout 53f407608ef6547bd690fa4420418aa6b6991b22 && \
    sed -i '/Building Bazel with Bazel/iexit 0' compile.sh && \
    ./compile.sh compile && \
    ln -s /opt/bazel/output/bazel /usr/bin/
