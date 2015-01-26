#!/bin/bash -e

# Copyright 2014 Google Inc. All rights reserved.
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

DIR="$(readlink -e "$(dirname "$0")")"

CAMPFIRE_ROOT="$PWD"
while [[ ! -f "${CAMPFIRE_ROOT}/.campfire_settings" ]] && [[ "$CAMPFIRE_ROOT" != "/" ]]; do
  CAMPFIRE_ROOT="$(readlink -e "${CAMPFIRE_ROOT}/..")"
done
if [[ "$CAMPFIRE_ROOT" == "/" ]]; then
  echo "Could not find campfire root!" >&2
  exit 1
fi

cd "${CAMPFIRE_ROOT}"
SERVER='//kythe/release/appengine/xrefs:server'
./campfire build "$SERVER"
./campfire query "outputs(['$SERVER'])" | \
  jq -r '.[]' | xargs -I '{}' cp --preserve=all '{}' "$DIR"/

cd kythe/web/ui
lein cljsbuild once prod
cp -r --preserve=all resources/public "$DIR"/

cd "$DIR"
rm -f appengine_generated*
DOCKER_HOST=unix:///var/run/docker.sock gcloud preview app deploy --server preview.appengine.google.com .
