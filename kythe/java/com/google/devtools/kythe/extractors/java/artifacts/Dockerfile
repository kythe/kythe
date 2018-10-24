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

# Build: bazel build //kythe/java/com/google/devtools/kythe/extractors/java/artifacts
# Usage:
#   This container houses java extraction artifacts which can be drawn upon in an a la
#   carte fashion for building customized extraction images.
#
FROM debian:jessie

ADD kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac-wrapper.sh /opt/kythe/extractors/javac-wrapper.sh
ADD kythe/java/com/google/devtools/kythe/extractors/java/standalone/javac_extractor_deploy.jar /opt/kythe/extractors/javac_extractor.jar
ADD third_party/javac/javac-9-dev-r4023-1.jar /opt/kythe/extractors/javac9_tools.jar
