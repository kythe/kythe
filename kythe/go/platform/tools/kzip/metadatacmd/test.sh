#!/bin/bash
# Copyright 2020 The Kythe Authors. All rights reserved.
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

set -e

# For the test, we use a constant string for the time. When working with a git
# repository, the timestamp can be retrieved with `git log --pretty='%ad' -n 1
# HEAD`. The format '%ad' prints out just the 'author date' of the commit. The
# format of the date can be changed with git's --date flag, but the kzip
# create_metadata command is setup to accept git's default format.
TIMESTAMP="Thu Jun 4 23:15:09 2020 -0700"

echo "Commit timestamp: $TIMESTAMP"

$KZIP create_metadata \
    --output meta.kzip \
    --corpus test_corpus \
    --commit_timestamp "$TIMESTAMP"

$KZIP view meta.kzip | $JQ .
