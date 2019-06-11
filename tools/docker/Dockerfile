# Copyright 2015 The Kythe Authors. All rights reserved.
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

# docker build -t gcr.io/kythe-repo/kythe-builder .
FROM ubuntu:xenial

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends curl ca-certificates && \
    apt-get clean

RUN echo "deb [arch=amd64] http://storage.googleapis.com/bazel-apt stable jdk1.8" | tee /etc/apt/sources.list.d/bazel.list
RUN curl https://bazel.build/bazel-release.pub.gpg | apt-key add -

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      git pkg-config zip unzip rsync patch zsh wget net-tools less parallel locales make \
      g++ gcc openjdk-8-jdk openjdk-8-source clang-3.6 flex asciidoc source-highlight graphviz \
      zlib1g-dev libarchive-dev uuid-dev bazel \
      ca-certificates-java libsasl2-dev && \
    apt-get clean

RUN update-alternatives --set java /usr/lib/jvm/java-8-openjdk-amd64/jre/bin/java
ENV JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
ENV CC=/usr/bin/clang-3.6

# Bison
RUN wget http://archive.kernel.org/debian-archive/debian/pool/main/b/bison/bison_2.3.dfsg-5_amd64.deb -O /tmp/bison_2.3.deb && \
    dpkg -i /tmp/bison_2.3.deb && \
    rm -f /tmp/bison_2.3.deb

# Go
RUN wget https://storage.googleapis.com/golang/go1.4.linux-amd64.tar.gz -O /tmp/go.tar.gz && \
    tar xzf /tmp/go.tar.gz -C /usr/local/ && \
    rm -f /tmp/go.tar.gz
ENV PATH=/usr/local/go/bin:$PATH
