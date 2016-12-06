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
