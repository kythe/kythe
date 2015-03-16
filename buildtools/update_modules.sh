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
ROOT="$(dirname $0)/.."
cd "${ROOT}"
ROOT_ABS="${PWD}"
NODEDIR=${NODEDIR:-$ROOT_ABS/third_party/node/bin/}
QUERY_CONFIG="${NODEDIR}/node ${ROOT_ABS}/buildtools/main query_config"
LLVM_REPO="$(${QUERY_CONFIG} root_rel_llvm_repo)"
CAMPFIRE_CXX="$(${QUERY_CONFIG} cxx_path)"
CAMPFIRE_CC="$(${QUERY_CONFIG} cc_path)"
mkdir -p "${LLVM_REPO}"
cd "${LLVM_REPO}"
LLVM_REPO_ABS="${PWD}"
cd "${ROOT_ABS}"
. "./buildtools/module_versions.sh"
if [[ -z "$1" || "$1" == "--git_only" || "$1" == "--docker" ]]; then
  echo "Using repository in ${LLVM_REPO_ABS} from root ${ROOT_ABS}"
  if [ ! -d "${LLVM_REPO}/.git" ]; then
    git clone http://llvm.org/git/llvm.git "${LLVM_REPO_ABS}"
  fi
  if [ ! -d "${LLVM_REPO}/tools/clang/.git" ]; then
    git clone http://llvm.org/git/clang.git "${LLVM_REPO_ABS}/tools/clang"
  fi
  if [ ! -d "${LLVM_REPO}/tools/clang/tools/extra/.git" ]; then
    git clone http://llvm.org/git/clang-tools-extra.git \
        "${LLVM_REPO_ABS}/tools/clang/tools/extra"
  fi
  cd "${LLVM_REPO_ABS}"
  git checkout master
  git pull
  git checkout "${MIN_LLVM_SHA}"
  cd tools/clang
  git checkout master
  git pull
  git checkout "${MIN_CLANG_SHA}"
  cd tools/extra
  git checkout master
  git pull
  git checkout "${MIN_EXTRA_SHA}"
fi
if [[ "$1" == "--docker" ]]; then
  cd "${ROOT_ABS}"
  ./campfire-docker build_update_modules
fi
if [[ -z "$1" || "$1" == "--build_only" ]]; then
  mkdir -p "${LLVM_REPO_ABS}/build"
  cd "${LLVM_REPO_ABS}/build"
  ../configure CC="${CAMPFIRE_CC}" CXX="${CAMPFIRE_CXX}" \
                --prefix="${LLVM_REPO_ABS}/build-install" \
                CXXFLAGS="-std=c++11" \
                --enable-optimized --disable-bindings
  make -j8
  cd "${ROOT_ABS}"
  mkdir -p third_party/llvm/include
  ./kythe/cxx/extractor/rebuild_resources.sh \
      third_party/llvm/llvm/build/Release+Asserts/lib/clang/3.7.0 > \
      third_party/llvm/include/cxx_extractor_resources.inc
fi
