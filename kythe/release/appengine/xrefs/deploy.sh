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

#
# Builds serving tables based on Kythe's sources, if not present at ./serving
# already, and deploys an xrefs server with the sample UI to appengine.
# Arguments passed to this script are passed directly to
# ./build_serving_tables.sh except for the optional --dev argument that creates
# a local dev server instead of deploying.
#
# Usage: ./deploy.sh [--dev] [build_serving_tables.sh args]

DIR="$(readlink -e "$(dirname "$0")")"

DEV=
if [[ "$1" == "--dev" ]]; then
  DEV=1
  shift
fi

if [[ ! -d "$DIR"/serving ]]; then
  "$DIR"/build_serving_tables.sh "$@" --out "$DIR"/serving
fi

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
rsync -aP --delete resources/public "$DIR"/

cd "$DIR"
rm -f appengine_generated*

if [[ -n "$DEV" ]]; then
  dev_appserver.py .
else
  gcloud preview app deploy --server gcr.appengine.google.com .
fi

