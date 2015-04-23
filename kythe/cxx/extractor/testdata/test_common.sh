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
# TODO(zarko): remove "campfire", lift this script out for use in
# other test suites.
KYTHE_BIN="${TEST_SRCDIR:-${PWD}/campfire-out/bin}"
BASE_DIR="${TEST_SRCDIR:-${PWD}}/kythe/cxx/extractor/testdata"
OUT_DIR="${TEST_TMPDIR:-${PWD}/campfire-out/test/kythe/cxx/extractor/testdata/${TEST_NAME:?No TEST_NAME}}"
mkdir -p "${OUT_DIR}"
EXTRACTOR="${KYTHE_BIN:-./campfire-out/bin}/kythe/cxx/extractor/cxx_extractor"
KINDEX_TOOL="${KYTHE_BIN:-./campfire-out/bin}/kythe/cxx/tools/kindex_tool"
VERIFIER="${KYTHE_BIN:-./campfire-out/bin}/kythe/cxx/verifier/verifier"
INDEXER="${KYTHE_BIN:-./campfire-out/bin}/kythe/cxx/indexer/cxx/indexer"
INDEXPACK="${KYTHE_BIN:-./campfire-out/bin}/kythe/go/platform/tools/indexpack"
