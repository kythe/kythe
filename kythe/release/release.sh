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

cd "$(dirname "$0")/../.." # campfire root

GCS_BUCKET=artifacts.kythe-repo.appspot.com

READERS=(
  700500404081@developer.gserviceaccount.com # jenkins-plugins
  712568501325@developer.gserviceaccount.com # p2d-maven-git
)

CONTAINERS=(
  //kythe/release
  //buildtools/docker
)

update_acls() {
  local users=()
  for reader in ${READERS[@]}; do
    users+=(-u "$reader:READ")
  done

  gsutil defacl ch "${users[@]}" "gs://$GCS_BUCKET"
  gsutil acl ch "${users[@]}" "gs://$GCS_BUCKET"
  gsutil -m acl ch -R "${users[@]}" "gs://$GCS_BUCKET"
}

#update_acls

gcloud preview docker --authorize_only
./campfire deploy ${CONTAINERS[@]}
