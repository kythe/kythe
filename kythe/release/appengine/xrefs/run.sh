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

LATEST_SCHEMA=$(gsutil ls gs://kythe-builds/schema_*/schema.html | tail -n1)
update_schema.sh "${LATEST_SCHEMA%schema.html}*"

GRAPHSTORES=$(gsutil ls gs://kythe-builds/graphstore_* | gawk '
{
  match($0, /graphstore(_(.+?))?_[0-9]{4}-/, m)
  latest[m[2]]=$0
}
END {
  for (k in latest) {
    print latest[k]
  }
}' | tr '\n' ',')

service nginx start
server --initial_stores "${GRAPHSTORES%,}" |& tee /var/log/server.log
