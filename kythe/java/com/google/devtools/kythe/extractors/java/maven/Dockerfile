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

# Build: bazel build //kythe/java/com/google/devtools/kythe/extractors/java/maven:docker
# Usage:
#   Maven normally does not use javac directly, but this container sets MAVEN_OPTS to force javac's
#   usage.  If your build requires other MAVEN_OPTS, be sure to keep the compiler settings intact so
#   that extracting a Maven build can be as simple as:
#     mkdir -p "$OUT"
#     docker run --rm -v "$REPO:/repo" -v "$OUT:/out" \
#       gcr.io/kythe_repo/kythe-maven-extractor mvn clean compile
#
#   NOTE: you may want to utilize the /root/.m2 volume to save time downloading your dependencies

FROM gcr.io/kythe_repo/kythe-javac-extractor

RUN apt-get update && \
    apt-get install -y --no-install-recommends maven && \
    apt-get clean

VOLUME /root/.m2

ENV MAVEN_OPTS="-Dmaven.compiler.forceJavacCompilerUse=true -Dmaven.compiler.fork=true -Dmaven.compiler.executable=$JAVA_HOME/bin/javac"
