#!/bin/sh -e
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
# Lists the SHA (for git) and REV (for svn) for each required component.
# LLVM checkouts can be svn or git. Each svn revision has an associated
# git-svn sha. (The svn revision will appear in the git log entry.) When a
# checkout is updated to a new minimum version, both its _SHA and _REV should
# be set to the new (matched) strings.

# llvm
MIN_LLVM_SHA="1df0e84c254981ffb57f526fae4c903112760a44"
MIN_LLVM_REV="250805"
# clang
MIN_CLANG_SHA="8a7f6787710fa0fbc06293f51856bac19dfd6b7e"
MIN_CLANG_REV="250804"
# clang-tools-extra
MIN_EXTRA_SHA="ed971c0759e8a5145b699e12ca24dfad79a5e19a"
MIN_EXTRA_REV="250742"

FULL_SHA="${MIN_LLVM_SHA}-${MIN_CLANG_SHA}-${MIN_EXTRA_SHA}"
