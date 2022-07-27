#!/bin/bash
# Copyright 2021 The Kythe Authors. All rights reserved.
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

RUNFILES_DIR="$0.runfiles"

# We have to call find ourselves because rlocation doesn't support wildcards
PATTERN="$RUNFILES_DIR/rust_*_x86_64*_tools/lib/rustlib*lib/librustc_driver-*.*"
# Find the Rust directory first so that our next find doesn't search the
# entire runfiles directory
RUST_RUNFILES_LOCATION="$(find "$RUNFILES_DIR" -type d -wholename "$RUNFILES_DIR/rust_*_x86_64*_tools")"
LOCATION="$(find "$RUST_RUNFILES_LOCATION" -wholename "$PATTERN")"
LIB_DIRNAME="$(dirname "$LOCATION")"

# Set the dylib search paths for Linux and macOS, respectively
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}$LIB_DIRNAME"
export DYLD_FALLBACK_LIBRARY_PATH="${DYLD_FALLBACK_LIBRARY_PATH:+$DYLD_FALLBACK_LIBRARY_PATH:}$LIB_DIRNAME"

exec "$RUNFILES_DIR/io_kythe/kythe/rust/extractor/extractor" "$@"
