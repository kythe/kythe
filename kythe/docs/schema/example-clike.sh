#!/bin/bash -e
set -o pipefail

# Copyright 2016 Google Inc. All rights reserved.
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

# This script formats a single Kythe example, which is expected to be piped in
# on standard input from example.sh. It will not run the verifier but will
# format the example like C++.
#
# The script assumes its working directory is the schema output directory and
# requires the following environment variables:
#   TMP
#   LANGUAGE
#   LABEL

SRCS="$TMP/example"
mkdir "$SRCS"
ARGS_FILE="$TMP/args"
touch "$ARGS_FILE"

# The raw filter input will be placed into this file for later syntax
# highlighting.
RAW_EXAMPLE="$TMP/raw.hcc"
cat > "${RAW_EXAMPLE}"

echo "<div><h5 id=\"_${LABEL}\">${LABEL}"
echo " (${LANGUAGE})</h5>"
source-highlight --failsafe --output=STDOUT --src-lang cpp -i "$RAW_EXAMPLE"
echo "</div>"
