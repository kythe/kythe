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

FROM buildkite/agent:3-ubuntu

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
      # Setup dependencies
      gcc git curl wget \
      # Required by go tool build of Kythe
      libleveldb-dev libbrotli-dev \
      # rules_go dependencies \
      patch \
      # Kythe C++ dependencies
      clang uuid-dev flex bison \
      # Kythe misc local tool dependencies
      asciidoc source-highlight graphviz parallel && \
    apt-get clean

# Install Go
RUN curl -L https://dl.google.com/go/go1.11.5.linux-amd64.tar.gz | tar -C /usr/local -xz

ENV PATH=$PATH:/usr/local/go/bin

# Install bazelisk.
RUN go get github.com/philwo/bazelisk && \
  ln -s /root/go/bin/bazelisk /usr/local/bin/bazel

ADD bazelrc /root/.bazelrc
