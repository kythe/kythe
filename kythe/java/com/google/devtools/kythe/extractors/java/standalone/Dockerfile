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

# Build: ./campfire package //kythe/java/com/google/devtools/kythe/extractors/java/standalone:docker
# Usage:
#   This container has OpenJDK8 installed with a small wrapper replacing the javac executable.  When
#   invoked, it will first extract the compilation information, placing the resulting .kindex file
#   in the $KYTHE_OUTPUT_DIRECTORY (defaults to the /out volume), and then run the real javac
#   compiler.  This allows Kythe to instrument any build system using javac directly.
#
#   The wrapper script assumes that the root of source repository that is being built is the /repo
#   volume.  If it is not, KYTHE_ROOT_DIRECTORY must be updated accordingly.  Likewise, the other
#   javac extractor environment variables can be passed to configure the extraction.  See
#   //kythe/java/com/google/devtools/kythe/extractors/java/standalone/AbstractJavacWrapper.java for
#   more information.
#
#   NOTE: This container is meant to be extended to support build systems like Maven or Bazel.
FROM debian:jessie

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y --no-install-recommends curl ca-certificates && \
    apt-get clean

RUN curl https://storage.googleapis.com/dev-con-jessie-apt/convoy.key | apt-key add -
RUN echo deb http://storage.googleapis.com/dev-con-jessie-apt/ jessie main >> /etc/apt/sources.list

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
      openjdk-8-jdk openjdk-8-source ca-certificates-java && \
    apt-get clean

ENV JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64

VOLUME /repo
VOLUME /out

WORKDIR /repo
ENV KYTHE_ROOT_DIRECTORY=/repo
ENV KYTHE_OUTPUT_DIRECTORY=/out

ADD fix_permissions.sh /usr/bin/fix_permissions
ADD javac_extractor_deploy.jar /opt/

RUN mv $JAVA_HOME/bin/javac $JAVA_HOME/bin/javac.real
ADD javac-wrapper.sh $JAVA_HOME/bin/javac
