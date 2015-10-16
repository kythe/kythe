# Copyright 2014 Google Inc. All rights reserved.
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

ENV DEBIAN_FRONTEND noninteractive

RUN echo "deb http://http.debian.net/debian jessie-backports main" >> /etc/apt/sources.list

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
      curl ca-certificates \
      less parallel jq net-tools locales zsh wget unzip libssl-dev \
      openjdk-8-jdk clang-3.5 ca-certificates-java uuid-dev flex \
      libmemcached-dev libsasl2-dev && \
    apt-get clean

# Maven
ENV MAVEN_VERSION 3.1.0
RUN wget http://archive.apache.org/dist/maven/binaries/apache-maven-$MAVEN_VERSION-bin.zip && \
    unzip apache-maven-$MAVEN_VERSION-bin.zip && \
    rm apache-maven-$MAVEN_VERSION-bin.zip
ENV PATH /apache-maven-$MAVEN_VERSION/bin:$PATH

# Bison
RUN wget http://archive.kernel.org/debian-archive/debian/pool/main/b/bison/bison_2.3.dfsg-5_amd64.deb -O /tmp/bison_2.3.deb && \
    dpkg -i /tmp/bison_2.3.deb && \
    rm -f /tmp/bison_2.3.deb

# Go
RUN wget https://storage.googleapis.com/golang/go1.4.linux-amd64.tar.gz -O /tmp/go.tar.gz && \
    tar xzf /tmp/go.tar.gz -C /usr/local/ && \
    rm -f /tmp/go.tar.gz
ENV PATH=/usr/local/go/bin:$PATH

# fix HOME=/ issue in old Docker versions
ENV HOME /root

# Setup Kythe directory structure
RUN mkdir -p /kythe/{bin,lib}/
ENV PATH /kythe/bin:$PATH

ADD kythe/release/base/fix_permissions.sh /kythe/bin/fix_permissions
