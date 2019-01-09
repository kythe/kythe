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

FROM gcr.io/cloud-builders/go:debian

RUN apt-get update && \
    apt-get install -y parallel && \
    apt-get clean

ADD kythe/go/extractors/cmd/gotool/analyze_packages.sh /usr/local/bin/analyze_packages.sh
ADD kythe/go/extractors/cmd/gotool/gotool /usr/local/bin/extract_go
ADD kythe/go/platform/tools/kzip/kzip /usr/local/bin/
ADD kythe/release/base/fix_permissions.sh /usr/local/bin/

ENTRYPOINT ["analyze_packages.sh"]
