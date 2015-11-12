#!/bin/bash -e

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

# This script builds the TypeScript indexer and executes it on its own code,
# sending output to `self-entries.json`. It should be run from //kythe/ts,
# and you should have run `npm i` before calling it.

set +o pipefail
if [[ ! -f ./node_modules/.bin/gulp ]]; then
  echo 'This script expects to be run from //kythe/ts; it also expects'
  echo 'that you have run "npm i".'
  exit 1
fi

./node_modules/.bin/gulp 1>&2

KYTHE_VNAMES=./../data/vnames.json \
node build/lib/main.js --rootDir ./lib --skipDefaultLib \
    -- typings/**/*.d.ts lib/*.ts \
    > self-entries.json
