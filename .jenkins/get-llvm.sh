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

readonly GCS_BUCKET="gs://kythe-builds"

usage() {
  echo "usage: $(basename "$0") cache-dir" >&2
}

if [[ $# != 1 ]]; then
  usage
  exit 1
fi

CACHE="$(readlink -m "$1")"
echo "Cache Root:      $CACHE"

REPO="$(readlink -e "$(dirname "$0")/..")"
echo "Repository Root: $REPO"

. tools/modules/versions.sh
NAME="llvm-${FULL_SHA}"
LLVM="$CACHE/$NAME"

if [[ ! -d "$LLVM" ]]; then
  TMP="$(mktemp -d)"
  trap "rm -rf '$TMP'" EXIT ERR INT
  cd "$TMP"
  gsutil cp "${GCS_BUCKET}/$NAME.tar.gz" .
  tar xzf "$NAME.tar.gz"
  mkdir -p "$(dirname "$LLVM")"
  mv llvm "$LLVM"
else
  echo "Reusing cached LLVM version at $LLVM"
fi

rm -rf "$REPO"/third_party/llvm/llvm
cp -r "$LLVM" "$REPO"/third_party/llvm/llvm
