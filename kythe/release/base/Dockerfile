# Copyright 2014 The Kythe Authors. All rights reserved.
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

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends \
      curl ca-certificates \
      less parallel jq net-tools locales zsh wget unzip libssl-dev \
      openjdk-8-jdk clang-3.8 ca-certificates-java uuid-dev flex \
      maven bison golang \
      libsasl2-dev && \
    apt-get clean

# Setup Kythe directory structure
RUN mkdir -p /kythe/{bin,lib}/
ENV PATH /kythe/bin:$PATH

ADD kythe/release/base/fix_permissions.sh /kythe/bin/fix_permissions
