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
#
# Translate Bazel's special XCode parameters into paths based on the
# environment variables `SDKROOT` and `DEVELOPER_DIR` before invoking
# the C++ extractor.

# --- begin runfiles.bash initialization ---
set -uo pipefail; f=bazel_tools/tools/bash/runfiles/runfiles.bash
# shellcheck disable=SC1090
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null || \
source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null || \
source "$0.runfiles/$f" 2>/dev/null || \
source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null || \
{ echo>&2 "ERROR: cannot find $f"; exit 1; }; f=; set -e
# --- end runfiles.bash initialization ---

# We have to call find ourselves because rlocation doesn't support wildcards
PATTERN="$RUNFILES_DIR/rust_*_x86_64/lib/rustlib*lib/librustc_driver-*.so"
# Find the Rust directory first so that our next find doesn't search the
# entire runfiles directory
RUST_RUNFILES_LOCATION="$(find "$RUNFILES_DIR" -type d -wholename "$RUNFILES_DIR/rust_*_x86_64")"
LOCATION="$(find "$RUST_RUNFILES_LOCATION" -wholename "$PATTERN")"
LIB_DIRNAME="$(dirname "$LOCATION")"

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+$LD_LIBRARY_PATH:}$LIB_DIRNAME"
exec "$(rlocation io_kythe/kythe/rust/extractor/extractor)" "$@"
