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
# This script checks that required libraries have been checked out at the
# versions we expect.

LLVM_REPO='third_party/llvm/llvm'
. "$(dirname "$0")/versions.sh"

cd "$(dirname "$0")/../.."
ROOT="$PWD"

# check_repo repo_path friendly_name expect_sha expect_rev
check_repo() {
  ( cd "${1:?no repo path}" && [[ -d ".git" ]] \
          && git merge-base --is-ancestor "${3:?no SHA}" HEAD 2>/dev/null \
          && cd "$ROOT" ) \
      || ( cd "$1" && [[ -d ".svn" ]] \
          && [[ $(svnversion) -ge "${4:?no revison}" ]] \
          && cd "$ROOT" ) \
      || ( echo \
            "Missing ${2:-repo checkout} with ancestor $3 (rev $4) in $1
Please run ./tools/modules/update.sh." \
          && exit 1 )
}

check_repo "${LLVM_REPO}" "LLVM" "${MIN_LLVM_SHA}" "${MIN_LLVM_REV}"
check_repo "${LLVM_REPO}/tools/clang" "clang" "${MIN_CLANG_SHA}" \
     "${MIN_CLANG_REV}"
