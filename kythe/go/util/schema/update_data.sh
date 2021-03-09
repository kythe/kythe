#!/bin/sh
# Copyright 2021 The Kythe Authors. All rights reserved.
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

set -e
readonly output=kythe/go/util/schema/indexdata.go

echo "-- Updating $output from Bazel ... " 1>&2
set -x
bazel build //kythe/go/util/schema:schema_index
cp bazel-genfiles/kythe/go/util/schema/schema_index.go "$output"
chmod 0644 "$output"
