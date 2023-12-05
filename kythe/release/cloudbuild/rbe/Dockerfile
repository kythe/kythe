# Copyright 2023 The Kythe Authors. All rights reserved.
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
FROM ubuntu:23.04

# RBE build tools
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y && \
    apt-get install -y \
      binutils \
      ca-certificates \
      curl \
      ed \
      file \
      git \
      less \
      locales \
      locales-all \
      netcat-traditional \
      openssh-client \
      patch \
      unzip \
      wget \
      zip && \
    apt-get clean

# RBE Clang packages
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y && \
    apt-get install -y \
      libstdc++-11-dev \
      libc++-dev \
      clang && \
    apt-get clean

# RBE Go packages
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y && \
    apt-get install -y \
      golang && \
    apt-get clean

# RBE Java packages
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y && \
    apt-get install -y \
      ca-certificates-java \
      openjdk-11-jre-headless \
      openjdk-11-jdk-headless && \
    apt-get clean

# RBE Python packages
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y && \
    apt-get install -y \
      python3-dev \
      python3-setuptools \
      software-properties-common && \
    apt-get clean

# Kythe dependencies
RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y && \
    apt-get install -y \
      uuid-dev flex bison make \
      asciidoc ruby-dev source-highlight graphviz && \
    apt-get clean

ENV LANG=C.UTF-8 LANGUAGE=C.UTF-8 LC_ALL=C.UTF-8

# Needed by rbe_config_gen
ENV JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
