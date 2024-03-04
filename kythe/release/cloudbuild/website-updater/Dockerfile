# Copyright 2024 The Kythe Authors. All rights reserved.
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

FROM gcr.io/kythe-repo/bazelisk-builder-client

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y \
    asciidoc asciidoctor source-highlight graphviz ruby-dev build-essential \
    rsync jq

# install `gcloud` cli tool
RUN curl -O https://dl.google.com/dl/cloudsdk/channels/rapid/downloads/google-cloud-cli-466.0.0-linux-x86_64.tar.gz
RUN tar -xf google-cloud-cli-466.0.0-linux-x86_64.tar.gz
RUN ./google-cloud-sdk/install.sh
ENV PATH="${PATH}:/google-cloud-sdk/bin"
RUN which gcloud

# install extra components needed for website deployment
RUN gcloud components install app-engine-go cloud-datastore-emulator app-engine-python-extras
