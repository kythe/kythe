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
# Script to encapsulate ./tools/modules/update.sh with a local cache directory.
#
# Usage: cache-llvm.sh <--check|--restore|--save|--init|--update>
#
#   --init: creates a temporary directory of the Kythe repo, builds the latest
#           required LLVM version, and saves it in the LLVM cache
#   --check: exits successfully if the exact required version of LLVM/clang is
#            checked out
#   --restore: restores the required version of LLVM from the cache, if available
#   --save: copies the currently built version of LLVM to the cache
#   --update: restores the required version of LLVM from the cache, if available.
#             If unavailable, LLVM is built at the necessary version and saved
#             in the cache.
#
# Note: `cache-llvm.sh --update` captures 99.99% of use-cases (just use that)

CACHE="$HOME/.cache/kythe-llvm"

cache_location() {
  if [[ ! -r ./tools/modules/versions.sh ]]; then
    echo "ERROR: could not find ./tools/modules/versions.sh; are you in the Kythe repo root?" >&2
    exit 1
  fi
  source tools/modules/versions.sh
  echo "$CACHE/$FULL_SHA"
}

check_version() {
  source tools/modules/versions.sh
  local cwd="$PWD"
  pushd third_party/llvm/llvm
  if [[ "$(git rev-parse HEAD)" != "$MIN_LLVM_SHA" ]]; then
    echo "LLVM version differs" >&2
    cd "$cwd"
    return 1
  fi
  pushd tools/clang
  if [[ "$(git rev-parse HEAD)" != "$MIN_CLANG_SHA" ]]; then
    echo "Clang version differs" >&2
    cd "$cwd"
    return 1
  fi
  cd "$cwd"
}

save_cache() {
  local dir
  dir="$(cache_location)"
  if [[ ! -d "$dir" ]]; then
    echo "Caching LLVM as $dir"
    mkdir -p "$CACHE"
    cp -al third_party/llvm/llvm "$dir"
  fi
}

restore_cache() {
  if ! check_version &>/dev/null; then
    local dir
    dir="$(cache_location)"
    if [[ -d "$dir" ]]; then
      echo "Restoring LLVM from $dir"
      rm -rf third_party/llvm/llvm
      cp -al "$dir" third_party/llvm/llvm
    else
      echo "Could not restore LLVM" >&2
      return 1
    fi
  fi
}

case "$1" in
  --check)
    check_version ;;
  --restore)
    restore_cache ;;
  --save)
    save_cache ;;
  --update)
    if ! restore_cache; then
      ./tools/modules/update.sh
      save_cache
    fi ;;
  --init)
    TMP="$(mktemp -d)"
    cd "$TMP"

    echo "Building LLVM in $TMP"
    git clone https://github.com/kythe/kythe.git
    cd kythe
    ./tools/modules/update.sh

    save_cache
    rm -rf "$TMP" ;;
  *)
    echo "Usage: cache-llvm.sh <--check|--restore|--save|--init|--update>"
    exit 1
esac

