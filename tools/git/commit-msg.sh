#!/bin/bash -e
#
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

# Check commit message: https://www.conventionalcommits.org/en/v1.0.0-beta.2/
bazel run \
  --ui_event_filters=-info \
  --show_result=0 \
  --noshow_progress \
  --noshow_loading_progress \
  --run_under "cd '$PWD' && " @io_kythe//tools/git:commitlint -- --edit "$1"
