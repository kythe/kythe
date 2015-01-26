#!/bin/bash -e
#
# Copyright 2014 Google Inc. All rights reserved.
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

# Builds website, serves it locally, and checks it for any broken links.

cd "$(dirname "$0")"

./build.sh
bundle exec jekyll serve --skip-initial-build &
server_pid=$!
trap 'kill "$server_pid"' EXIT ERR INT

while ! curl -s localhost:4000 >/dev/null; do
  echo 'Waiting for server...' >&2
  sleep 1s
done

echo
set -o pipefail
wget --spider -nv -e robots=off -r -p http://localhost:4000 2>&1 | \
  grep -v -e 'unlink: ' -e ' URL:'
