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

cd "$(dirname $0)/../.."
ROOT="$PWD"

bazel build //tools/modules:compiler_info
eval "$(<bazel-genfiles/tools/modules/compiler_info.txt)"

LLVM_REPO="$ROOT/third_party/llvm/llvm"
mkdir -p "$LLVM_REPO"

if [[ -d "$LLVM_REPO/build" && ! -h "$LLVM_REPO/build" ]]; then
  echo "Your checkout has a directory at:"
  echo "  $LLVM_REPO/build"
  echo "update_modules expects this to be a symlink that it will overwrite."
  echo "Please remove this directory if you want update_modules to manage LLVM."
  exit 1
fi

. ./tools/modules/versions.sh
if [[ -z "$1" || "$1" == "--git_only" ]]; then
  echo "Using repository in $LLVM_REPO"

  git_maybe_clone http://llvm.org/git/llvm.git "$LLVM_REPO"
  git_maybe_clone http://llvm.org/git/clang.git "$LLVM_REPO/tools/clang"
  git_maybe_clone http://llvm.org/git/clang-tools-extra.git \
    "$LLVM_REPO/tools/clang/tools/extra"

  git_checkout_sha "$LLVM_REPO" "$MIN_LLVM_SHA"
  git_checkout_sha "$LLVM_REPO/tools/clang" "$MIN_CLANG_SHA"
  git_checkout_sha "$LLVM_REPO/tools/clang/tools/extra" "$MIN_EXTRA_SHA"
fi

if [[ -z "$1" || "$1" == "--build_only" ]]; then
  cd "$LLVM_REPO"
  vbuild_dir="build.${MIN_LLVM_SHA}.${MIN_CLANG_SHA}.${MIN_EXTRA_SHA}"
  if [[ ! -d "$vbuild_dir" ]]; then
    mkdir -p "$vbuild_dir"
    trap "rm -rf '$LLVM_REPO/$vbuild_dir'" ERR INT
    cd "$vbuild_dir"
    CXX=$(basename "${BAZEL_CC}" | sed -E 's/(cc)?(-.*)?$/++\2/')
    if [ ! -z $(dirname "${BAZEL_CC}") ]; then
      CXX="$(dirname "${BAZEL_CC}")/${CXX}"
    fi
    ../configure CC="${BAZEL_CC}" CXX="${CXX}" \
      --prefix="$LLVM_REPO/build-install" \
      CXXFLAGS="-std=c++11" \
      --enable-optimized --disable-bindings
    make -j8
    cd ..
  fi
  rm -f build
  if [[ $(uname) == 'Darwin' ]]; then
    ln -sf "$vbuild_dir" build
  else
    ln -sfT "$vbuild_dir" build
  fi
fi
