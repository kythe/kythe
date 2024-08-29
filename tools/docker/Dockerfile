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
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends curl ca-certificates && \
    apt-get clean

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      git pkg-config zip unzip rsync patch zsh wget net-tools less parallel locales make \
      g++ gcc openjdk-21-jdk openjdk-21-source clang-15 flex asciidoc source-highlight graphviz \
      zlib1g-dev libarchive-dev uuid-dev golang bison \
      ca-certificates-java libsasl2-dev && \
    apt-get clean

RUN update-alternatives --set java /usr/lib/jvm/java-21-openjdk-amd64/bin/java
ENV JAVA_HOME=/usr/lib/jvm/java-21-openjdk-amd64
ENV CC=/usr/bin/clang-15

# Go
ENV PATH=/root/go/bin:$PATH
