#!/bin/bash -e
# Copyright 2017 Google Inc. All rights reserved.
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

echo "Downloading latest serving data..." >&2
gsutil cp gs://kythe_serving/latest.tar.gz .
echo "Unpacking serving data..." >&2
tar -C /srv/ -xzf latest.tar.gz
rm -rf latest.tar.gz

echo "Launching server..." >&2
exec /usr/local/bin/server --listen :8080
