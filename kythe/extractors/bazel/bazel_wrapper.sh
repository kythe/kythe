#!/bin/bash

# Copyright 2019 The Kythe Authors. All rights reserved.
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

# Usage:
# Invoke as you would bazel.
#
# If bazel builds all targets successfully, or fails to buid some targets this
# script will exit with code 0. If bazel fails for some other reason, this
# script will exit with code 1.

set -x

# It is ok if targets fail to build. We build using --keep_going and don't
# care if some targets fail, but bazel will return a failure code if any
# targets fail. We *do* care if bazel exits with an error code other than some
# targets failed to build.
/kythe/bazelisk "$@"
RETVAL=$?
if [[ $RETVAL -eq 1 ]]; then
    echo "Not all bazel targets built successfully, but continuing anyways."
    exit 0
elif [[ $RETVAL -ne 0 ]]; then
    echo "Bazel build failed with exit code: $RETVAL"
    exit 1
fi
