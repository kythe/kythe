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

set -o pipefail
#
# usage: update_graphstore.sh gs://<bucket>/<graphstore_path> <path>
# example: update_graphstore.sh gs://kythe-builds/graphstore_2014-09-25_20-28-22.tar.gz /srv/xrefs/gs1
#
# This script updates the given serving path with the given graphstore archive on GCS.

GCS="$1"
SRV="$2"

mkdir -p "$(dirname "$SRV")"

TMP=$(mktemp -d)
cleanup() {
  rm -rf "$TMP"
}
trap cleanup EXIT

gsutil cp "$GCS" "$TMP/"
mkdir "$TMP/store"
tar xf "$TMP/"graphstore_* -C "$TMP/store"
chown www-data:www-data -R "$TMP/store"
rm -rf "$SRV"
mv "$TMP/store" "$SRV"
