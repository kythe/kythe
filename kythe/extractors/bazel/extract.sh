#!/bin/bash

# Copyright 2019 The Kythe Authors. All rights reserved.
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

# Usage:
# extract.sh \
#     --define kythe_corpus=github.com/project/repo \
#     //...
#
# Input `kythe_corpus` should be the repository you're extracting (for example
# github.com/protocolbuffers/protobuf), as well as the build target to extract.
# A good default is //..., which extracts almost everything.
#
# Note that this doesn't some targets - for example build targets with
# `tags = ["manual"]` will not be extracted.  If you have  additional targets to
# extract those can be appended cleanly:
#
#     extract.sh --define... //... //some/manual:target
#
# If you have restrictions on what can or should be extracted, for example an
# entire directory to ignore, you must specify your partitions individually.  A
# good tool for doing this (instead of manually discovering everything) is bazel
# query, described at
# https://docs.bazel.build/versions/master/query-how-to.html.
#
# Outputs $KYTHE_OUTPUT_DIRECTORY/compilations.kzip
#
# Requires having environment variable $KYTHE_OUTPUT_DIRECTORY set, as well
# as kzip tool (kythe/go/platform/tools/kzip) installed to /kythe/kzip and
# kythe/release/base/fix_permissions.sh copied to /kythe/fix_permissions.sh.
# Also assumes you have extractors installed as per
# kythe/extractors/bazel/extractors.bazelrc.

# Print our commands for easier debugging and exit after our first failed
# command so we avoid silent failures.
set -ex

: ${KYTHE_OUTPUT_DIRECTORY:?Missing output directory}

if [ -n "$KYTHE_SYSTEM_DEPS" ]; then
  echo "Installing $KYTHE_SYSTEM_DEPS"
  # TODO(jaysachs): unclear if we should bail if any packages fail to install
  apt-get update && \
  apt-get upgrade -y && \
  apt-get install -y $KYTHE_SYSTEM_DEPS && \
  apt-get clean
fi

if [ -n "$KYTHE_PRE_BUILD_STEP" ]; then
  eval "$KYTHE_PRE_BUILD_STEP"
fi

# It is ok if targets fail to build. We build using --keep_going and don't
# care if some targets fail, but bazel will return a failure code if any
# targets fail.
/kythe/bazelisk --bazelrc=/kythe/bazelrc "$@" || true
retval=$?
if [ $retval -eq 1 ]; then
    echo "Not all bazel targets built successfully, but continuing anyways."
else if [ $retval -ne 0 ];
    echo "Bazel build failed with exit code: $retval"
    exit 1
fi

# Collect any extracted compilations.
mkdir -p "$KYTHE_OUTPUT_DIRECTORY"
find bazel-out/*/extra_actions/external/kythe_release -name '*.kzip' | \
  xargs -r /kythe/tools/kzip merge --append --output "$KYTHE_OUTPUT_DIRECTORY/compilations.kzip"
/kythe/fix_permissions.sh "$KYTHE_OUTPUT_DIRECTORY"
test -f "$KYTHE_OUTPUT_DIRECTORY/compilations.kzip"
