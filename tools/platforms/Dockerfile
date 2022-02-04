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

# docker build -t gcr.io/kythe-repo/kythe-builder .
FROM gcr.io/cloud-marketplace/google/rbe-ubuntu18-04@sha256:48b67b41118dbcdfc265e7335f454fbefa62681ab8d47200971fc7a52fb32054

RUN DEBIAN_FRONTEND=noninteractive \
    apt-get update -y && \
    apt-get install -y \
      # Kythe C++ dependencies
      uuid-dev flex bison make \
      # Kythe misc dependencies
      asciidoc ruby-dev source-highlight graphviz && \
    apt-get clean

# bazel-toolchains 5+ pulls the JDK from the image's JAVA_HOME
# which defaults to JDK8 and breaks.
# See https://github.com/bazelbuild/bazel-toolchains/issues/961
ENV JAVA_HOME=/usr/lib/jvm/11.29.3-ca-jdk11.0.2/reduced

# Ensure JAVA_HOME exists.
RUN [[ -d "$JAVA_HOME" ]]
