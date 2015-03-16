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
MIN_LLVM_SHA="ed0266d8ee16537e7cec9d9409ddf07a8e3efbc5"
MIN_LLVM_REV="231571"
# clang
MIN_CLANG_SHA="9e51c85a582963e272b3462d5f40752aff2830ab"
MIN_CLANG_REV="231564"
# clang-tools-extra
MIN_EXTRA_SHA="82e3f4fb0b2898ec2c581d0fe26f52f48d535003"
MIN_EXTRA_REV="231440"
