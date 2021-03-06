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

FROM ubuntu:focal

RUN apt-get update && \
    apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
      # Buildbot and pre-commit dependencies
      python python3 python3-dev python3-pip wget git \
      # Required by go tool build of Kythe
      golang-go libleveldb-dev libbrotli-dev \
      # Bazel's fallback @local_jdk
      openjdk-11-jdk \
      # Bazel dependencies
      zip zlib1g-dev unzip \
      # Kythe C++ dependencies
      clang-11 uuid-dev flex bison \
      # Linter dependencies
      shellcheck clang-format-11 \
      # Kythe misc dependencies
      asciidoc ruby-dev source-highlight graphviz curl parallel jq && \
    apt-get clean

# Make clang-11 the default
RUN update-alternatives \
      --install /usr/bin/clang        clang        /usr/bin/clang-11    100 \
      --slave   /usr/bin/clang++      clang++      /usr/bin/clang++-11      \
      --slave   /usr/bin/clang-format clang-format /usr/bin/clang-format-11

# Install pip-packages
RUN pip3 install --upgrade pip
RUN pip3 install pre-commit
RUN pip3 install buildbot
RUN pip3 install buildbot-www buildbot-console-view buildbot-grid-view buildbot-waterfall-view psycopg2-binary txrequests
RUN pip3 install --upgrade six service_identity pyasn1 cryptography pyopenssl
RUN pip3 install buildbot-worker

# Install extra linters
RUN go get github.com/bazelbuild/buildtools/buildifier
RUN go get golang.org/x/lint/golint
RUN go get honnef.co/go/tools/cmd/staticcheck
ENV PATH=$PATH:/root/go/bin
RUN wget https://github.com/google/google-java-format/releases/download/google-java-format-1.9/google-java-format-1.9-all-deps.jar && \
   mv google-java-format-1.9-all-deps.jar /usr/bin/google-java-format.jar && \
   /bin/echo -e '#!/bin/sh\nexec java -jar /usr/bin/google-java-format.jar "$@"' >/usr/bin/google-java-format && \
   chmod +x /usr/bin/google-java-format

# Install Bazelisk and wrapper script
ADD bazel /usr/bin/bazel
RUN curl -L -o /usr/bin/bazelisk https://github.com/bazelbuild/bazelisk/releases/download/v1.0/bazelisk-linux-amd64 && chmod +x /usr/bin/bazelisk

# Install go wrapper script
ADD go /usr/local/bin/go

# Buildbot configuration
ADD bazelrc /root/.bazelrc
ADD start.sh /buildbot/
ADD worker /buildbot/worker
ADD master /buildbot/master

ADD secrets.tar /buildbot
RUN buildbot checkconfig /buildbot/master/master.cfg

EXPOSE 8080
CMD /buildbot/start.sh
