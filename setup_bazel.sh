#!/bin/sh -e

# Copyright 2015 Google Inc. All rights reserved.
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

# Initializes the Bazel workspace.  Uses the JAVA_HOME and GOROOT environmental
# variables.  If either is unset, the paths will be determined by
# "$(which java)/.." and "$(which go)/..", respectively.

cd "$(dirname "$0")"

by_bin() {
  echo "$(readlink -e "$(dirname "$(which "$1")")/..")"
}

if [ -z "$JAVA_HOME" ]; then
  JAVA_HOME="$(by_bin java)"
fi

if [ -z "$GOROOT" ]; then
  GOROOT="$(by_bin go)"
fi

echo "Using jdk found in $JAVA_HOME" >&2
ln -sTf "$JAVA_HOME" tools/jdk/jdk

echo "Using go found in $GOROOT" >&2
ln -sTf "$GOROOT" tools/go/go
