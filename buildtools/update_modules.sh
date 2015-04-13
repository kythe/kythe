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
#
# update_modules.sh updates the repositories for external code (to the precise
# revisions used for testing).
#
# If the single argument `--git_only` is passed, the repositories will be
# updated to the pinned versions without configuring or building them.
# If the argument `--build_only` is passed, the script will assume that the
# repositories are at the correct version and will configure and build them.
#
# If the argument `--docker` is passed, the dependencies will be fetched on
# the host, but the build will take place using the campfire-docker toolchain.

git_maybe_clone() {
  local repo="$1"
  local dir="$2"
  if [[ ! -d "$dir/.git" ]]; then
    git clone "$repo" "$dir"
  fi
}

git_checkout_sha() {
  local repo="$1"
  local sha="$2"
  cd "$repo"
  if [[ "$sha" != "$(git rev-parse HEAD)" ]]; then
    git fetch origin master
    git checkout -f "$sha"
  fi
  cd - >/dev/null
}

ROOT_REL="$(dirname $0)/.."
cd "$ROOT_REL"
ROOT="$PWD"

NODEDIR="${NODEDIR:-"$ROOT/third_party/node/bin/"}"
query_config() {
  "$NODEDIR/node" "$ROOT/buildtools/main" query_config "$@"
}

CAMPFIRE_CXX="$(query_config cxx_path)"
CAMPFIRE_CC="$(query_config cc_path)"

LLVM_REPO_REL="$(query_config root_rel_llvm_repo)"
mkdir -p "$LLVM_REPO_REL"
LLVM_REPO="$(readlink -e "$LLVM_REPO_REL")"

. "./buildtools/module_versions.sh"
if [[ -z "$1" || "$1" == "--git_only" || "$1" == "--docker" ]]; then
  echo "Using repository in $LLVM_REPO_REL (relative to $ROOT_REL)"

  git_maybe_clone http://llvm.org/git/llvm.git "$LLVM_REPO"
  git_maybe_clone http://llvm.org/git/clang.git "$LLVM_REPO/tools/clang"
  git_maybe_clone http://llvm.org/git/clang-tools-extra.git \
    "$LLVM_REPO/tools/clang/tools/extra"

  git_checkout_sha "$LLVM_REPO" "$MIN_LLVM_SHA"
  git_checkout_sha "$LLVM_REPO/tools/clang" "$MIN_CLANG_SHA"
  git_checkout_sha "$LLVM_REPO/tools/clang/tools/extra" "$MIN_EXTRA_SHA"
fi

if [[ "$1" == "--docker" ]]; then
  ./campfire-docker build_update_modules
fi

if [[ -z "$1" || "$1" == "--build_only" ]]; then
  vbuild_dir="$LLVM_REPO/build.${MIN_LLVM_SHA}.${MIN_CLANG_SHA}.${MIN_EXTRA_SHA}"
  if [[ ! -d "$vbuild_dir" ]]; then
    mkdir -p "$vbuild_dir"
    trap "rm -rf '$vbuild_dir'" ERR INT
    cd "$vbuild_dir"
    ../configure CC="${CAMPFIRE_CC}" CXX="${CAMPFIRE_CXX}" \
      --prefix="$LLVM_REPO/build-install" \
      CXXFLAGS="-std=c++11" \
      --enable-optimized --disable-bindings
    make -j8
  fi
  ln -sf "$vbuild_dir" "$LLVM_REPO/build"
fi
