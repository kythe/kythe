# Copyright 2019 The Kythe Authors. All rights reserved.
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

FROM gcr.io/cloud-builders/bazel

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
      # Install C++ compilers for cc_* rule support.
      gcc clang && \
    apt-get clean

# Kythe extractor binaries
ADD external/io_kythe_lang_proto/kythe/go/extractors/proto/extract_proto_kzip /kythe/extractors/bazel_proto_extractor
ADD kythe/cxx/extractor/cxx_extractor_bazel /kythe/extractors/bazel_cxx_extractor
ADD kythe/go/extractors/cmd/bazel/bazel_go_extractor /kythe/extractors/bazel_go_extractor
ADD kythe/go/extractors/cmd/bazel/extract_kzip /kythe/extractors/bazel_extract_kzip
ADD kythe/java/com/google/devtools/kythe/extractors/java/bazel/java_extractor_deploy.jar /kythe/extractors/bazel_java_extractor.jar
ADD kythe/java/com/google/devtools/kythe/extractors/jvm/bazel/bazel_jvm_extractor_deploy.jar /kythe/extractors/bazel_jvm_extractor.jar

# Tools and configuration
ADD kythe/data/* /kythe/
ADD kythe/extractors/bazel/extract.sh /kythe/
ADD kythe/extractors/extractors.bzl /kythe/
ADD kythe/go/platform/tools/kzip/kzip /kythe/
ADD kythe/release/base/fix_permissions.sh /kythe/

# Bazel repository setup
ADD kythe/extractors/bazel/extractors.BUILD /kythe/BUILD
ADD kythe/extractors/bazel/extractors.WORKSPACE /kythe/WORKSPACE
ADD kythe/extractors/bazel/extractors.bazelrc /kythe/bazelrc
RUN cat /kythe/bazelrc > ~/.bazelrc

ENTRYPOINT ["/kythe/extract.sh"]
