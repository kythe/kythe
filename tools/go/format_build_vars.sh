#!/bin/bash -e

# Copyright 2016 Google Inc. All rights reserved.
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

# Formats each status variable from bazel-out/{stable,volatile}-status.txt as a
# `go link tool` -X flag.  Used primarily in //tools/build_rules:go.bzl.
#
# Usage: format_build_vars <build-package>

readonly BUILD_VAR_PACKAGE="$1"

if [[ -z "$BUILD_VAR_PACKAGE" ]]; then
  echo "ERROR: missing required <build-package> argument" >&2
  exit 1
fi

awk -v "pkg=$BUILD_VAR_PACKAGE" '{
  printf "-X %s._%s=", pkg, $1;
  $1="";
  print substr($0, 2);
}' bazel-out/{stable,volatile}-status.txt

