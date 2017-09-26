#!/bin/bash -e

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

# Script to extract compilations from a Bazel repository.  See usage() below.

usage() {
  cat >&2 <<EOF
Usage: $(basename "$0") [--index_pack] [--java] [--cxx] [--bazel_arg arg] [bazel-root] <output-dir>

Run the Kythe extractors as Bazel extra actions and copy the outputs to a given directory.  By
default, both C++ and Java are extracted, but if --java or --cxx are given, then the set of
extractors used are determined by the flags given.  In other words, giving both --java and --cxx is
the same as no flags.

Flags:
  --index_pack: use <output-dir> as the root of an index pack instead of a collection of .kindex files
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
INDEX_PACK=
BAZEL_ARGS=(--bazelrc=/dev/null build -k)

while [[ $# -gt 0 ]]; do
  case "$1" in
    --help|-h)
      usage ;;
    --index_pack)
      INDEX_PACK=1 ;;
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
  BAZEL_ARGS+=(--experimental_action_listener=//kythe/java/com/google/devtools/kythe/extractors/java/bazel:extract_kindex)
fi
if [[ -n "$CXX" ]]; then
  BAZEL_ARGS+=(--experimental_action_listener=//kythe/cxx/extractor:extract_kindex)
fi
if [[ -n "$GO" ]]; then
  BAZEL_ARGS+=(--experimental_action_listener=//kythe/go/extractors/cmd/bazel:extract_kindex_go)
fi

set -x
bazel "${BAZEL_ARGS[@]}" //... || \
  echo "Bazel exited with error code $?; trying to continue..." >&2
set +x

xad="$(find -L bazel-out -type d -name extra_actions)"
INDICES=($(find "$xad" -name '*.kindex'))
echo "Found ${#INDICES[@]} .kindex files in $xad"
for idx in "${INDICES[@]}"; do
  name="$(basename "$idx" .kindex)"
  lang="${name##*.}"
  dir="$OUTPUT/$lang"

  if [[ -n "$INDEX_PACK" ]]; then
      "$ROOT/bazel-bin/kythe/go/platform/tools/indexpack/indexpack" --to_archive "$dir" "$idx"
  else
    mkdir -p "$dir"
    dest="$dir/$(tr '/' '_' <<<"${idx#$xad/}")"
    cp -vf "$idx" "$dest"
  fi
done
