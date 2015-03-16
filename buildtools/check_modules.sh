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
# check_modules.sh config_query_cmd
#
# This script uses config_query_cmd to pull configuration variables from
# .campfire_settings. It then checks that dependent libraries have been
# checked out at the versions we expect.
# Remember that it is possible to override .campfire_settings properties
# with a .campfire_settings file in your home directory, like so:
# {
#   "configurations": {
#     "base": {
#       "third_party_llvm_rel_llvm_lib": "/some/path",
#       "root_rel_llvm_include": ["/some/path", "/another/path"],
#       "root_rel_llvm_repo": "/some/path"
#     }
#   }
# }
# third_party_llvm_rel_llvm_lib is relative to third_party/llvm.
# root_rel_llvm_repo is relative to the campfire root.
# root_rel_llvm_include is also relative to the campfire root.
# Any of these properties may be set to absolute paths, as in the example
# above.
QUERY_CONFIG="$1"
LLVM_LIB=$(${QUERY_CONFIG} third_party_llvm_rel_llvm_lib)
LLVM_INCLUDE=$(${QUERY_CONFIG} root_rel_llvm_include)
LLVM_REPO="$(${QUERY_CONFIG} root_rel_llvm_repo)"
CWD="${PWD}"

. "$(dirname $0)/module_versions.sh"

# check_repo repo_path friendly_name expect_sha expect_rev
check_repo() {
  ( cd "${1:?no repo path}" && [[ -d ".git" ]] \
          && git merge-base --is-ancestor "${3:?no SHA}" HEAD 2>/dev/null \
          && cd "${CWD}" ) \
      || ( cd "$1" && [[ -d ".svn" ]] \
          && [[ $(svnversion) -ge "${4:?no revison}" ]] \
          && cd "${CWD}" ) \
      || ( echo \
            "Missing ${2:-repo checkout} with ancestor $3 (rev $4) in $1
Please see README.adoc for details (or run ./buildtools/update_modules.sh or
./buildtools/update_modules.sh --docker from your campfire root)" \
          && exit 1 )
}

check_repo "${LLVM_REPO}" "LLVM" "${MIN_LLVM_SHA}" "${MIN_LLVM_REV}"
check_repo "${LLVM_REPO}/tools/clang" "clang" "${MIN_CLANG_SHA}" \
     "${MIN_CLANG_REV}"
check_repo "${LLVM_REPO}/tools/clang/tools/extra" "clang extra tools" \
     "${MIN_EXTRA_SHA}" "${MIN_EXTRA_REV}"

if [ ! -e "$(dirname $0)/../third_party/llvm/include/cxx_extractor_resources.inc" ]; then
  echo 'Missing amalgamated header for C++. Please run from the project root:
mkdir -p third_party/llvm/include && cd third_party/llvm && \
../../kythe/cxx/extractor/rebuild_resources.sh \
  $(./../../campfire query_config third_party_llvm_rel_llvm_lib)/clang/3.7.0 > \
  ./include/cxx_extractor_resources.inc && cd ../..'
  exit 1
fi
