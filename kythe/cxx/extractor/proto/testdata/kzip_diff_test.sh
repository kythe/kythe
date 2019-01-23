#!/bin/bash
# Copyright 2018 The Kythe Authors. All rights reserved.
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

# Output every line except ones that contains match_string. Also, do not output
# extra_lines_to_skip lines that follow the line that contained match_string.
function skip() {
  if [ $# -ne 3 ] && [ $# -ne 2 ]; then
    echo "Usage: $0 match_string extra_lines_to_skip [input_file]"
    return 1
  fi
  local -r match_string="$1"
  local -r lines_to_skip="$2"

  if [ $# -eq 2 ]; then
    awk "/$match_string/{skip=$lines_to_skip;next} skip>0{--skip;next} {print}"
  else
    awk "/$match_string/{skip=$lines_to_skip;next} skip>0{--skip;next} {print}" "$3"
  fi
  return 0
}

# Modify input_file to remove lines that contains match_string. Also remove
# extra_lines_to_skip lines that follow the line that contained match_string.
function skip_inplace() {
  if [ $# -ne 3 ]; then
    echo "Usage: $0 match_string lines_to_skip input_file"
    return 1
  fi
  local -r match_string="$1"
  local -r lines_to_skip="$2"
  local -r input_file="$3"

  local -r temp_file=$(mktemp)
  skip "$match_string" "$lines_to_skip" "$input_file" > "$temp_file"
  mv "$temp_file" "$input_file"
}


set -e

KINDEX_TOOL=$1; shift
KZIP=$1; shift
GOLDEN=$1; shift

# Work in tmpdir because the runfiles dir is not writable
cp $KZIP "${TEST_TMPDIR}/file.kzip"
# Extract textproto version of compilation unit from kzip.
"${KINDEX_TOOL}" -canonicalize_hashes -suppress_details -explode "${TEST_TMPDIR}/file.kzip"
UNIT="${TEST_TMPDIR}/file.kzip_UNIT"

# Remove lines that will change depending on the machine the test is run on.
skip_inplace "-target" 1 $UNIT
skip_inplace "signature" 0 $UNIT
skip_inplace "working_directory" 0 $UNIT

echo
echo "Diffing generated unit against golden"
echo "-------------------------------------"
diff -u $GOLDEN $UNIT
echo "No diffs found!"
