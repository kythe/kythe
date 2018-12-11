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
#
# update_modules.sh updates the repositories for external code (to the precise
# revisions used for testing).
#
# If the single argument `--fetch_only` is passed, the repositories will be
# updated to the pinned versions without configuring or building them.
# If the argument `--build_only` is passed, the script will assume that the
# repositories are at the correct version and will configure and build them.

# This section is used for cleanup during/after fetching.
ZIPFILES=()
TMPDIRS=()

wget_cleanup() {
  ZIPFILES+=("$1")
  TMPDIRS+=("${2?:missing directory}")
  trap trap_cleanup EXIT ERR INT
}

trap_cleanup() {
  cd "$ROOT"
  for file in "${ZIPFILES[@]}"; do
    rm "$file"
  done
  for dir in "${TMPDIRS[@]}"; do
    rm -rf "$dir"
  done
}

wget_copy_archive() {
  # The dependency to download from llvm-mirror (llvm, clang).
  local target="$1"
  # The directory to download the dependency into.
  local dir="${2:?missing directory}"
  # The specific version of the dependency to use.
  local sha="$3"

  if [[ ! -f "$dir/$sha.sentinel" ]]; then
    rm -rf "$dir"
    wget "https://github.com/llvm-mirror/$target/archive/$sha.zip"
    local tmpdir=$(mktemp -d)
    wget_cleanup $sha.zip $tmpdir
    unzip "$sha.zip" -d "$tmpdir/"
    # This relies on the behavior of github to always produce a zip archive with
    # a subdirectory named "repo-###sha###" that contains the repo inside it.
    mv "$tmpdir/$target-$sha" "$dir"
    # Leave an empty file so that we know what version we have checked out.
    touch "$dir/$sha.sentinel"
  fi
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
if [[ -z "$1" || "$1" == "--fetch_only" ]]; then
  echo "Using repository in $LLVM_REPO"

  wget_copy_archive "llvm" "$LLVM_REPO" "$MIN_LLVM_SHA"
  wget_copy_archive "clang" "$LLVM_REPO/tools/clang" "$MIN_CLANG_SHA"
  rm -rf "$LLVM_REPO/tools/clang/tools/extra"
  # Do cleanup, and clear the trap for later use.
  trap_cleanup
  trap - EXIT ERR INT
fi

if [[ -z "$1" || "$1" == "--build_only" ]]; then
  cd "$LLVM_REPO"
  vbuild_dir="build.${MIN_LLVM_SHA}.${MIN_CLANG_SHA}"
  find "${LLVM_REPO}" -maxdepth 1 -type d \
      ! -name "${vbuild_dir}" -name 'build.*.*' \
      -exec rm -rf \{} \;
  if [[ ! -d "$vbuild_dir" ]]; then
    mkdir -p "$vbuild_dir"
    llvm_cleanup
    trap "rm -rf '$LLVM_REPO/$vbuild_dir'" ERR INT
    cd "$vbuild_dir"
    CXX=$(basename "${BAZEL_CC}" | sed -E 's/(cc)?(-.*)?$/++\2/')
    if [ ! -z $(dirname "${BAZEL_CC}") ]; then
      CXX="$(dirname "${BAZEL_CC}")/${CXX}"
    fi
    if [ ! -x "$CXX" ]; then
      CXX="$BAZEL_CC"  # Fall back to unadorned compiler name.
    fi
    if [[ $(uname) == 'Darwin' ]]; then
      CMAKE_CXX_FLAGS="-lstdc++"
    fi
    CMAKE_GEN="Unix Makefiles"
    if which ninja > /dev/null 2> /dev/null; then
      CMAKE_GEN=Ninja
    fi
    cmake "-G$CMAKE_GEN" \
        -DCMAKE_INSTALL_PREFIX="$LLVM_REPO/build-install" \
        -DCMAKE_BUILD_TYPE="Release" \
        -DCMAKE_C_COMPILER="${BAZEL_CC}" \
        -DCMAKE_CXX_COMPILER="${CXX}" \
        -DCLANG_BUILD_TOOLS="OFF" \
        -DCLANG_INCLUDE_DOCS="OFF" \
        -DCLANG_INCLUDE_TESTS="OFF" \
        -DLIBCLANG_BUILD_STATIC="ON" \
        -DLLVM_BUILD_TOOLS="OFF" \
        -DLLVM_BUILD_UTILS="OFF" \
        -DLLVM_BUILD_RUNTIME="OFF" \
        -DLLVM_DYLIB_COMPONENTS="" \
        -DLLVM_ENABLE_OCAMLDOC="OFF" \
        -DLLVM_ENABLE_RTTI="ON" \
        -DLLVM_INCLUDE_DOCS="OFF" \
        -DLLVM_INCLUDE_EXAMPLES="OFF" \
        -DLLVM_INCLUDE_GO_TESTS="OFF" \
        -DLLVM_INCLUDE_TESTS="OFF" \
        -DLLVM_INCLUDE_TOOLS="ON" \
        -DLLVM_INCLUDE_UTILS="OFF" \
        -DLLVM_TOOL_CLANG_TOOLS_EXTRA_BUILD="OFF" \
        -DLLVM_TARGETS_TO_BUILD="X86;PowerPC;ARM;AArch64;Mips" \
        -DBUILD_SHARED_LIBS="OFF" \
        -DLLVM_BUILD_LLVM_DYLIB="OFF" \
        -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}" \
        ..
    if [[ $(uname) == 'Darwin' ]]; then
      cores="$(sysctl -n hw.ncpu)"
    else
      cores="$(nproc)"
    fi
    cmake --build . -- "-j${cores}" \
        clangAnalysis clangAST clangBasic clangDriver clangEdit clangToolingInclusions \
        clangFrontend clang-headers clangLex clangParse clangRewrite clangSema \
        clangSerialization clangTooling LLVMAArch64Info LLVMARMInfo \
        LLVMBitReader LLVMCore LLVMMC LLVMMCParser LLVMMipsInfo LLVMOption \
        LLVMBinaryFormat clangIndex \
        LLVMPowerPCInfo LLVMProfileData LLVMX86Info clangFormat clangToolingCore
    cd ..
  fi
  rm -f build
  if [[ $(uname) == 'Darwin' ]]; then
    ln -sf "$vbuild_dir" build
  else
    ln -sfT "$vbuild_dir" build
  fi
fi
