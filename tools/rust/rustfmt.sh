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

find kythe/rust/ tools/rust/ -name '*.rs' -not -wholename "*target/*" -not -wholename "*indexer/testdata/*" -print0 | while read -r -d $'\0' f
do
    echo "Formatting $f";
    rustfmt --config-path "$(dirname "${BASH_SOURCE[0]}")" "$@" "$f";
done
