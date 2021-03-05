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

cd "$(dirname "$0")"/../..

setup_hook() {
  local hook="$1"
  local path=".git/hooks/$hook"
  local script="tools/git/${hook}.sh"
  local stmt="if [[ -r '$script' ]]; then './$script' \"\$@\"; fi"

  if [[ ! -r "$path" ]]; then
    echo "Creating $path" >&2
    echo "#!/bin/bash -e" > "$path"
    echo "$stmt" >> "$path"
    chmod +x "$path"
  elif ! grep -q "$script" "$path"; then
    echo "Appending $script to $path" >&2
    echo "$stmt" >> "$path"
  else
    echo "$path already contains reference to $script" >&2
  fi
}

setup_hook commit-msg
pre-commit install -t pre-commit
pre-commit install -t pre-push
