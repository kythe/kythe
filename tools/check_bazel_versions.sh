#!/bin/bash -e
#
# Copyright 2019 The Kythe Authors. All rights reserved.
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

if [[ $# -ne 2 ]]; then
  echo "ERROR: expected <min_version> <max_version> arguments" >&2
  exit 2
fi

BZL_MIN="$1"
BZL_MAX="$2"

FOUND_MIN="$(<.bazelminversion)"
FOUND_MAX="$(<.bazelversion)"

if [[ "$FOUND_MIN" != "$BZL_MIN" ]]; then
  echo "ERROR: .bazelminversion ($FOUND_MIN) does not match MIN_VERSION in version.bzl ($BZL_MIN)" >&2
  exit 1
fi

if [[ "$FOUND_MAX" != "$BZL_MAX" ]]; then
  echo "ERROR: .bazelminversion ($FOUND_MAX) does not match MAX_VERSION in version.bzl ($BZL_MAX)" >&2
  exit 1
fi
