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
# Define TEST_NAME, then include as part of an extractor test.
# TODO(zarko): lift this script out for use in other test suites.
BASE_DIR="$PWD/kythe/cxx/extractor/testdata"
OUT_DIR="$TEST_TMPDIR/out"
mkdir -p "${OUT_DIR}"
# This needs to be relative (or else it must be fixed up in the resulting
# compilation units) because the extractor stores its invocation path in its
# output.
EXTRACTOR="./kythe/cxx/extractor/cxx_extractor"
KINDEX_TOOL="${PWD}/kythe/cxx/tools/kindex_tool"
VERIFIER="${PWD}/kythe/cxx/verifier/verifier"
INDEXER="${PWD}/kythe/cxx/indexer/cxx/indexer"
INDEXPACK="${PWD}/kythe/go/platform/tools/indexpack/indexpack"

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
