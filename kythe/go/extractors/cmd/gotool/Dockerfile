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

FROM gcr.io/cloud-builders/go:debian-1.20

RUN apt-get update && \
    apt-get install -y parallel && \
    apt-get clean

# Get a recent go release, remove the current system one so we don't have multiple version problems, and install the new release.
RUN curl -LO https://golang.org/dl/go1.20.6.linux-amd64.tar.gz && rm -rf /usr/local/go && tar -C /usr/local -xzf go1.20.6.linux-amd64.tar.gz

ADD kythe/go/extractors/cmd/gotool/analyze_packages.sh /usr/local/bin/analyze_packages.sh
ADD kythe/go/extractors/cmd/gotool/gotool /usr/local/bin/extract_go
ADD kythe/go/platform/tools/kzip/kzip /usr/local/bin/
ADD kythe/release/base/fix_permissions.sh /usr/local/bin/

# running extract_go ensures that any glibc (or other library) incompatibilities are found at image build time rather than later when the image is deployed.
RUN /usr/local/bin/extract_go --help

ENTRYPOINT ["analyze_packages.sh"]
