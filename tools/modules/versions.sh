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
MIN_LLVM_SHA="42f7ad099aa73695ea633c585da0a9848d6a730d"
MIN_LLVM_REV="334273"
# clang
# NOTE: when updating Clang, make sure to adjust the path in
# //third_party/llvm/BUILD:clang_builtin_headers_resources
MIN_CLANG_SHA="63624e51bdbda6731b3b4ce87ecc1789c9b85b5e"
MIN_CLANG_REV="334221"

FULL_SHA="${MIN_LLVM_SHA}-${MIN_CLANG_SHA}"
