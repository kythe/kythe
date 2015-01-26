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

#
# usage: update_schema.sh gs://<bucket>/<schema_files>
# example: update_schema.sh gs://kythe-builds/schema_2014-09-19_22-08-11/*
#
# This script updates the schema serving directory with the given GCS schema files.

SRV=/srv/schema
mkdir -p $SRV
chown www-data:www-data -R /srv

TMP=$(mktemp -d)
cleanup() {
  rm -rf "$TMP"
}
trap cleanup EXIT

gsutil -m cp $1 $TMP
rsync -r --delete "$TMP/" "$SRV/"
