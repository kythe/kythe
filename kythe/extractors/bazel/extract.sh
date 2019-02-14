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
# extract.sh //...
#
# Outputs $KYTHE_OUTPUT_DIRECTORY/compilations.kzip
#
# Requires having environment variable $KYTHE_OUTPUT_DIRECTORY set, as well
# as kzip tool (kythe/go/platform/tools/kzip) installed to /kythe/kzip and
# kythe/release/base/fix_permissions.sh copied to /kythe/fix_permissions.sh.
# Also assumes you have extractors installed as per
# kythe/extractors/bazel/extractors.bazelrc.

bazel "$@"

# Collect any extracted compilations.
mkdir -p $KYTHE_OUTPUT_DIRECTORY
find bazel-out/*/extra_actions/external/kythe_extractors -name '*.kzip' | \
  xargs /kythe/kzip merge --output $KYTHE_OUTPUT_DIRECTORY/compilations.kzip
/kythe/fix_permissions.sh $KYTHE_OUTPUT_DIRECTORY
