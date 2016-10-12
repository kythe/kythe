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

# parse_args should be inlined into another script.
# It sets $INDEXER_ARGS to an array of arguments to pass to the indexer,
# $VERIFIER_ARGS to an array of arguments to pass to the verifier, and
# $CLANG_ARGS to an array of arguments to pass to indexer to set upclang.
# It will also set $RESULTS_EXPECTED and $TEST_FILE.

# Usage (imposed on embedders):
# script test-file {--indexer argument | --clang argument |
#     --verifier argument | --expected (expectfailindex|expectfailverify)}*

TEST_FILE="$1"
INDEXER_ARGS=()
VERIFIER_ARGS=()
CLANG_ARGS=()
RESULTS_EXPECTED=''

while shift; [ $# -ne 0 ]; do
  case "$1" in
  --expected)
    RESULTS_EXPECTED="$2"
    shift
    ;;
  --indexer)
    INDEXER_ARGS+=("$2")
    shift
    ;;
  --verifier)
    VERIFIER_ARGS+=("$2")
    shift
    ;;
  --clang)
    CLANG_ARGS+=("$2")
    shift
    ;;
  '')
    # Drop empty arguments.
    ;;
  *)
    echo "Unknown argument ${1}"
    exit 1
  esac
done
