#!/bin/bash -e
#
# Copyright 2018 The Kythe Authors. All rights reserved.
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
#
# Download, build, and extract a set of Go packages.  Resulting compilations
# will be merged into a single .kzip archive in the "$OUTPUT" directory.

: "${TMPDIR:=/tmp}" "${OUTPUT:=/output}"

if [[ -z "${GOPATH}" ]]; then
  echo "GOPATH environment variable is required" >&2
  exit 1
fi
if [[ -z "${KYTHE_CORPUS}" ]]; then
  echo "KYTHE_CORPUS environment variable is required" >&2
  exit 1
fi

FLAGS=()
PACKAGES=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    -*)
      FLAGS+=("$1") ;;
    *)
      PACKAGES+=("$1") ;;
  esac
  shift
done

if [[ ${#PACKAGES[@]} -ne 1 ]]; then
  echo "Please specify exactly one go package to extract" >&2
  exit 1
fi
PACKAGE="${PACKAGES[0]}"

# golang build tags can optionally be specified with the `KYTHE_GO_BUILD_TAGS`
# env variable.
if [ -n "$KYTHE_GO_BUILD_TAGS" ]; then
  FLAGS+=( "--buildtags=$KYTHE_GO_BUILD_TAGS" )
fi

if [ -n "$KYTHE_PRE_BUILD_STEP" ]; then
  eval "$KYTHE_PRE_BUILD_STEP"
fi

if [ -n "$KYTHE_SKIP_MOD_VENDOR" ]; then
  if [ "$KYTHE_SKIP_MOD_VENDOR" == "1" ] || \
     [ "$KYTHE_SKIP_MOD_VENDOR" == "True" ] || \
     [ "$KYTHE_SKIP_MOD_VENDOR" == "true" ]; then
    echo "$PACKAGE: Skipping 'go mod vendor' due to KYTHE_SKIP_MOD_VENDOR=$KYTHE_SKIP_MOD_VENDOR" >&2
    SKIP_MOD_VENDOR=true
  fi
fi

if [ -z "$SKIP_MOD_VENDOR" ]; then
  echo "$PACKAGE: go mod vendor" >&2
  pushd "$GOPATH/src/$PACKAGE"
  go mod vendor
  popd
fi

echo "Extracting ${PACKAGE}" >&2
  extract_go --continue -v \
  --goroot="$(go env GOROOT)" \
  --output="$TMPDIR/out.kzip" \
  --use_default_corpus_for_deps \
  --use_default_corpus_for_stdlib \
  --corpus="$KYTHE_CORPUS" \
  "${FLAGS[@]}" \
  "${PACKAGE}/vendor/..." \
  "${PACKAGE}/..."

mkdir -p "$OUTPUT"
OUT="$OUTPUT/compilations.kzip"
if [[ -f "$OUT" ]]; then
  OUT="$(mktemp -p "$OUTPUT/" compilations.XXXXX.kzip)"
fi

# cd into the top-level git directory of our package and query git for the
# commit timestamp.
pushd "$(go env GOPATH)/src/${PACKAGE}"
TIMESTAMP="$(git log --pretty='%ad' -n 1 HEAD)"
popd

# Record the timestamp of the git commit in a metadata kzip.
kzip create_metadata \
  --output "$OUTPUT/buildmetadata.kzip" \
  --corpus "$KYTHE_CORPUS" \
  --commit_timestamp "$TIMESTAMP"

echo "Merging compilations into $OUT" >&2
kzip merge --encoding "$KYTHE_KZIP_ENCODING" --output "$OUT" "$TMPDIR/out.kzip" "$OUTPUT/buildmetadata.kzip"
fix_permissions.sh "$OUTPUT"
