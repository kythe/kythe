#!/bin/bash -e
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

# This is a utility script which wraps around the kindex_tool in order to
# manually generate a kindex file for a specified input source file. This
# can be useful for debugging downstream indexers.
#
# Usage Example:
# bazel build //kythe/cxx/tools:kindex_file_generator
# bazel run //kythe/cxx/tools:kindex_file_generator "//kythe/BUILD" "kythe/BUILD" \
# "kythe" "bazel" "$(readlink -f ./output_dir/)/kythe.build.kindex" \
# $(readlink -f ./kythe/BUILD)
#
# This script looks for the optional env vars:
#   SHASUM_TOOL: The path to the shasum_tool to be utilized.
#   KINDEX_TOOL: The path to the kindex_tool to be utilized.

## Helper Functions ##

# Generates a vname text proto string.
#
# @param $1 corpus
# @param $2 path
# @param $3 (optional) signature
# @param $4 (optional) language
generate_vname_proto()
{
  local corpus="$1"
  local path="$2"
  local signature="$3"
  local language="$4"
  
  echo " {"
  echo "  corpus:\"$corpus\""
  echo "  path:\"$path\""
  
  if [ ! -z "$signature" ]; then
    echo "  signature:\"$signature\""
  fi

  if [ ! -z "$language" ]; then
    echo "  language:\"$language\""
  fi

  echo "}"
}

# Generates a file info proto string.
#
# @param $1 input_file_path
generate_file_info_proto()
{
  local input_file_path="$1"
  local digest=$($SHASUM_TOOL "$input_file_path" | head -c 64)
  echo " {"
  echo "   path:\"$input_file_path\""
  echo "   digest:\"$digest\""
  echo "}"
}

# Generates the file input proto string for the specified source file.
#
# @param $1 input_file_path
# @param $2 corpus
generate_file_input_proto()
{
  local input_file_path="$1"
  local corpus="$2"
  
  local file_info=$(generate_file_info_proto "$input_file_path")
  local vname_proto=$(generate_vname_proto "$corpus" "$input_file_path")
  echo " {"
  echo "    v_name: $vname_proto"
  echo "    info: $file_info"
  echo "}"
}

# Generates a compilation unit proto string
#
# @param cpu_signature
# @param corpus
# @param language
# @param input_source_file
generate_cpu_proto()
{
  local cpu_signature="$1"
  local corpus="$2"
  local language="$3"
  local input_source_file="$4"

  local vname_proto=$(generate_vname_proto "$corpus" "$input_source_file" \
   "$cpu_signature" "$language")
  local file_input_proto=$(generate_file_input_proto "$input_source_file" "$corpus")
  
  echo "v_name: $vname_proto"
  echo "required_input: $file_input_proto"
}

# Generates a FileData proto string for the specified input file.
#
# @param $1 input_file_path
generate_file_data_proto()
{
  local input_file_path="$1"

  local file_info=$(generate_file_info_proto "$input_file_path")
  local content_bytes=$(sed 's/\\/\\\\/g' < "$input_file_path" | \
    sed 's/\"/\\\"/g')
  
  echo "content: \"$content_bytes\""
  echo "info: $file_info"
}

## Main ##

# Check for proper usage
if [ $# -lt "5" ]; then
   echo "Usage: bazel run kythe/cxx/tools/kindex_file_generator [cpu_signature] \
[cpu_path] [corpus] [language] [kindex_output_file] [input_source_file]"
   exit 1
fi

# Parse input args.
CPU_SIGNATURE="$1"
CPU_PATH="$2"
CORPUS="$3"
LANGUAGE="$4"
KINDEX_OUTPUT_FILE="$5"
INPUT_SOURCE_FILE="$6"

# If no env var exists for the shasum_tool, set a default path
: ${SHASUM_TOOL:=${PWD}/kythe/go/platform/tools/shasum_tool/shasum_tool}


# If no env var exists for the kindex_tool, set a default path
: ${KINDEX_TOOL:=${PWD}/kythe/cxx/tools/kindex_tool}

# Create temp files.
CPU_TMP_FILE=$(mktemp)
FILE_DATA_TMP_FILE=$(mktemp)

# Setup a trap for cleanup.
trap "rm $CPU_TMP_FILE; rm $FILE_DATA_TMP_FILE" SIGHUP SIGINT SIGTERM EXIT

# Generate the CompilationUnit text proto and store into a temp file.
CPU_PROTO_TEXT=$(generate_cpu_proto "$CPU_SIGNATURE" "$CORPUS" "$LANGUAGE" \
  "$CPU_PATH")
echo $CPU_PROTO_TEXT > $CPU_TMP_FILE

# Generate the FileData text proto and store into a temp file.
FILE_DATA_PROTO_TXT=$(generate_file_data_proto "$INPUT_SOURCE_FILE")
echo $FILE_DATA_PROTO_TXT > $FILE_DATA_TMP_FILE

# Execute the kindex tool.
$KINDEX_TOOL -assemble "$KINDEX_OUTPUT_FILE" $CPU_TMP_FILE \
  $FILE_DATA_TMP_FILE
