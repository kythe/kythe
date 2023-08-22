# Copyright 2020 The Kythe Authors. All rights reserved.
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

# docker build -t gcr.io/kythe-repo/pre-commit .
FROM gcr.io/kythe-repo/bazelisk-builder-client:latest

RUN apt-get update \
        && apt-get upgrade -y \
        && apt-get install -y --no-install-recommends \
                curl ca-certificates \
                # pre-commit dependencies
                python3 python3-dev python3-pip python3-venv \
                # Linter dependencies
                golang-1.19-go shellcheck clang-format-14 openjdk-11-jre-headless git \
        && apt-get clean \
        && rm -rf /var/lib/apt/lists/*

# Make clang-format-14 the default
RUN update-alternatives \
      --install /usr/bin/clang-format clang-format /usr/bin/clang-format-14    100

# Make go-1.19 the default
RUN update-alternatives \
      --install /usr/bin/go go /usr/lib/go-1.19/bin/go  100
RUN update-alternatives \
      --install /usr/bin/gofmt gofmt /usr/lib/go-1.19/bin/gofmt  100

# Install pip-packages
COPY requirements.txt pre-commit-requirements.txt
RUN python3 -m venv /tmp/venv
RUN /tmp/venv/bin/pip3 install --require-hashes -r pre-commit-requirements.txt \
 && rm pre-commit-requirements.txt

# Install extra linters
RUN go install github.com/bazelbuild/buildtools/buildifier@latest \
 && go install golang.org/x/lint/golint@latest \
 && go install honnef.co/go/tools/cmd/staticcheck@latest

# Fetch the latest version of google-java-format from GitHub
RUN curl -s https://api.github.com/repos/google/google-java-format/releases/latest \
        | sed -n '/browser_download_url/s/[^:]*:[^"]*\("[^"]*"\).*/url = \1/p' \
        | egrep 'google-java-format-[^-]*-all-deps.jar' \
        | curl -L -o /usr/bin/google-java-format.jar -K - \
        && /bin/echo -e '#!/bin/sh\nexec java -jar /usr/bin/google-java-format.jar "$@"' >/usr/bin/google-java-format \
        && chmod +x /usr/bin/google-java-format

# Install go wrapper script
ADD go /usr/local/bin/go
ENV PATH=$PATH:/root/go/bin:$CARGO_HOME/bin
ENTRYPOINT ["/tmp/venv/bin/pre-commit"]
