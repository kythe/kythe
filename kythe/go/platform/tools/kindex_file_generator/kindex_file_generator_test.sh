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

# This script tests the functionality of the kindex_file_generator.sh script.

## Helper Functions ##

# Cleans up temporary files
cleanup_temp_files()
{
  for tmp_file in "${TMP_FILES[@]}"
  do
    rm -f $tmp_file
  done
}

# An array for storing temp file paths.
TMP_FILES=()

# Generates a new temp file.
generate_temp_file()
{
  local tmp_file=$(mktemp)
  TMP_FILES+=($tmp_file)
  echo $tmp_file
}

## Test Main ##

# Setup a trap to cleanup temp files.
trap "cleanup_temp_files" SIGHUP SIGINT SIGTERM EXIT

# Create a temp output kindex file
TMP_KINDEX_OUTPUT_FILE=$(generate_temp_file)

# Setup the path to the test input source file.
SOURCE_INPUT_FILE="$PWD/kythe/testdata/BUILD"

# Verify the source input file exists
if [ ! -e "$SOURCE_INPUT_FILE" ];
then
  echo "FAILED: could not find test input file: $SOURCE_INPUT_FILE"
  exit 1
fi

# Execute the kindex file generator, using the root kythe build file as input
$PWD/kythe/go/platform/tools/kindex_file_generator/kindex_file_generator \
  --uri "kythe://kythe?lang=bazel?path=kythe/BUILD#//kythe/BUILD" \
  --output "$TMP_KINDEX_OUTPUT_FILE" \
  --source "$SOURCE_INPUT_FILE"

# Ensure the output file was populated
# We cannot use stat because different systems implement it differently and
# provide different command line options.
OUTPUT_FILE_SIZE=$(du -k "$TMP_KINDEX_OUTPUT_FILE" | cut -f1)
if [ ! $OUTPUT_FILE_SIZE -gt "0" ];
then
  echo "FAILED: output kindex out contains no bytes."
  exit 1
fi

# Validate the generated kindex file by exploding it
$PWD/kythe/cxx/tools/kindex_tool -explode $TMP_KINDEX_OUTPUT_FILE

# Verify the exploded UNIT file was created
if [ ! -e "${TMP_KINDEX_OUTPUT_FILE}_UNIT" ];
then
  echo "FAILED: Exploded kindex UNIT file not found"
  exit 1
fi
