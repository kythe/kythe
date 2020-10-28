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
FROM gcr.io/cloud-marketplace/google/rbe-ubuntu18-04@sha256:5703843282226b2f7db86afc0b1535f46d411fa72c1d540e1bd0f8064adb0bdb

RUN apt-get update && \
    apt-get install -y \
      # Kythe C++ dependencies
      uuid-dev flex bison g++ \
      # Kythe misc dependencies
      asciidoc source-highlight graphviz && \
    apt-get clean
