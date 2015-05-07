#!/bin/bash

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

readonly JAR="/opt/javac_extractor_deploy.jar"
readonly BIN="$(dirname "$(readlink -e "$0")")"

cleanup() {
  fix_permissions "$KYTHE_ROOT_DIRECTORY"
  fix_permissions "$KYTHE_OUTPUT_DIRECTORY"
}
if [[ -z "$NO_CLEANUP" ]]; then
  trap cleanup EXIT ERR INT
fi

"$BIN/java" -jar "$JAR" "$@"
"$BIN/javac.real" "$@"
