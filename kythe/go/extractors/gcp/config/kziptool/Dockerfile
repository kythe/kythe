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

# Build: bazel build //kythe/go/extractors/gcp/config/kziptool:artifacts
# Usage:
#   This container exposes the kzip tool, useful for combining multiple kzip
#   output files into a single kzip.
#
#   From Google Cloud Build:
#     - name: 'gcr.io/kythe-public/kzip-tools'
#       entrypoint: 'bash'
#       args: ['-c', '/opt/kythe/tools/kzip merge --output /some/output.kzip /input/files/*.kzip']

FROM launcher.gcr.io/google/ubuntu16_04

ADD kythe/go/platform/tools/kzip/kzip /opt/kythe/tools/kzip

CMD ["/opt/kythe/tools/kzip"]
