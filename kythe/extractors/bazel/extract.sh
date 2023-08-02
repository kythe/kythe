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

: "${KYTHE_OUTPUT_DIRECTORY:?Missing output directory}" "${KYTHE_KZIP_ENCODING:=JSON}"

if [[ -n "$KYTHE_SYSTEM_DEPS" ]]; then
  echo "Installing $KYTHE_SYSTEM_DEPS"
  # shellcheck disable=SC2086
  # TODO(jaysachs): unclear if we should bail if any packages fail to install
  apt-get update && \
  apt-get upgrade -y && \
  apt-get --fix-broken install -y && \
  apt-get install -y $KYTHE_SYSTEM_DEPS && \
  apt-get clean
fi

if [[ -n "$KYTHE_PRE_BUILD_STEP" ]]; then
  eval "$KYTHE_PRE_BUILD_STEP"
fi

KYTHE_RELEASE=/kythe

if [[ -n "$KYTHE_BAZEL_TARGET" ]]; then
  # $KYTHE_BAZEL_TARGET is unquoted because bazel_wrapper needs to see each
  # target expression in KYTHE_BAZEL_WRAPPER as individual arguments. For
  # example, if KYTHE_BAZEL_TARGET=//foo/... -//foo/test/..., bazel_wrapper
  # needs to see two valid target expressions (//foo/... and -//foo/test/...)
  # not one invalid target expression with white space
  # ("//foo/... -//foo/test/...").
  /kythe/bazel_wrapper.sh --bazelrc="$KYTHE_RELEASE/extractors.bazelrc" "$@" --override_repository "kythe_release=$KYTHE_RELEASE" -- "$KYTHE_BAZEL_TARGET"
else
  # If the user supplied a bazel query, execute it and run bazel, but we have to
  # shard the results to different bazel runs because the bazel command line
  # cannot take many arguments. Right now we build 30 targets at a time. We can
  # change this value or make it settable once we have more data on the
  # implications.
  /kythe/bazelisk query "$KYTHE_BAZEL_QUERY" | \
    xargs -t -L 30 /kythe/bazel_wrapper.sh --bazelrc=$KYTHE_RELEASE/extractors.bazelrc "$@" --override_repository kythe_release=$KYTHE_RELEASE --
fi

# Collect any extracted compilations.
mkdir -p "$KYTHE_OUTPUT_DIRECTORY"
find bazel-out/*/extra_actions/ -name '*.kzip' -print0 | \
  xargs --null -r /kythe/tools/kzip merge --append --encoding "$KYTHE_KZIP_ENCODING" --output "$KYTHE_OUTPUT_DIRECTORY/compilations.kzip"

# Record the timestamp of the git commit in a metadata kzip.
/kythe/tools/kzip create_metadata \
  --output buildmetadata.kzip \
  --corpus "$KYTHE_CORPUS" \
  --commit_timestamp "$(git log --pretty='%ad' -n 1 HEAD)"
/kythe/tools/kzip merge --append --encoding "$KYTHE_KZIP_ENCODING" --output "$KYTHE_OUTPUT_DIRECTORY/compilations.kzip" buildmetadata.kzip

/kythe/fix_permissions.sh "$KYTHE_OUTPUT_DIRECTORY"
test -f "$KYTHE_OUTPUT_DIRECTORY/compilations.kzip"
