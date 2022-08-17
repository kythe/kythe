#!/bin/bash

# Copyright 2020 The Kythe Authors. All rights reserved.
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

# Find the rustfmt binary that was distributed with rules_rust so that we don't
# have to rely on the system binary, which may be out of date. If rules_rust
# is not present (which usually occurs if we are running in Cloud Build), we
# use the version of rustfmt installed on the system.
if [[ -d "bazel-kythe/external/rules_rust" ]]; then
    RUSTFMT=$(find -L bazel-kythe/external -not \( -path "bazel-kythe/external/io_kythe" -prune \) -wholename "bazel-kythe/external/rust_*_x86_64*_tools/bin/rustfmt")
else
    RUSTFMT="rustfmt"
fi

# If no arguments are provided, format all Rust files. Otherwise, format the
# specified file.
if [[ $# -eq 0 ]] ; then
    find kythe/rust/ tools/rust/ -name '*.rs' -not -wholename "*target/*" -not -wholename "*rust/*/testdata/*" -print0 | while read -r -d $'\0' f
    do
        echo "Formatting $f";
        $RUSTFMT --config-path "$(dirname "${BASH_SOURCE[0]}")" "$@" "$f" &
    done
    wait
else
    $RUSTFMT --config-path "$(dirname "${BASH_SOURCE[0]}")" "$1"
fi
