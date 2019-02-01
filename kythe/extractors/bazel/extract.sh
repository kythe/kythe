#!/bin/bash -e

# Copyright 2015 The Kythe Authors. All rights reserved.
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

# Script to extract compilations from a Bazel repository.  See usage() below.

usage() {
  cat >&2 <<EOF
Usage: $(basename "$0") [--java] [--go] [--cxx] [--bazel_arg arg] [bazel-root] <output-dir>

Run the Kythe extractors as Bazel extra actions and copy the outputs to a given directory.  By
default, C++, Java, and Go are extracted, but if --java, --go, or --cxx are given, then the set of
extractors used are determined by the flags given.  In other words, giving --java, --go, and --cxx
is the same as no flags.

Flags:
  --cxx:        turn on the use of the Bazel/Kythe C++ extractor
  --go:         turn on the use of the Bazel/Kythe Go extractor
  --java:       turn on the use of the Bazel/Kythe Java extractor
  --bazel_arg:  add the given argument to Bazel during the build
EOF
  exit 1
}

ALL=1
JAVA=
CXX=
GO=
BAZEL_ARGS=(build -k --output_groups=compilation_outputs)

while [[ $# -gt 0 ]]; do
  case "$1" in
    --help|-h)
      usage ;;
    --java)
      ALL=
      JAVA=1 ;;
    --cxx)
      ALL=
      CXX=1 ;;
    --go)
      ALL=
      GO=1 ;;
    --bazel_arg)
      BAZEL_ARGS+=("$2")
      shift ;;
    *)
      break ;;
  esac
  shift
done

if [[ -n "$ALL" ]]; then
  JAVA=1
  CXX=1
  GO=1
fi

ROOT=$PWD
case $# in
  1)
    OUTPUT="$1" ;;
  2)
    ROOT="$1"
    OUTPUT="$2" ;;
  *)
    usage ;;
esac

mkdir -p "$OUTPUT"
OUTPUT="$(readlink -e "$OUTPUT")"

cd "$ROOT"
if [[ -d bazel-out ]]; then
  find -L bazel-out -type d -name extra_actions -exec rm -rf '{}' +
fi

if [[ -n "$JAVA" ]]; then
  BAZEL_ARGS+=(--experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_java)
fi
if [[ -n "$CXX" ]]; then
  BAZEL_ARGS+=(--experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_cxx)
fi
if [[ -n "$GO" ]]; then
  BAZEL_ARGS+=(--experimental_action_listener=@io_kythe//kythe/extractors:extract_kzip_go)
fi

set -x
bazel "${BAZEL_ARGS[@]}" //... || \
  echo "Bazel exited with error code $?; trying to continue..." >&2
set +x

xad="$(find -L bazel-out -type d -name extra_actions)"
INDICES=($(find "$xad" -name '*.kzip'))
echo "Found ${#INDICES[@]} .kzip files in $xad"
for idx in "${INDICES[@]}"; do
  name="$(basename "$idx" .kzip)"
  lang="${name##*.}"
  dir="$OUTPUT/$lang"

  mkdir -p "$dir"
  dest="$dir/$(tr '/' '_' <<<"${idx#$xad/}")"
  cp -vf "$idx" "$dest"
done
