#!/bin/bash -e

# Copyright 2014 The Kythe Authors. All rights reserved.
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

MODULE=default

cd "$(dirname "$0")"

DEFAULT="$(gcloud app versions list --format=json --hide-no-traffic --service "$MODULE" 2>/dev/null | \
  jq -r '.[].id')"

echo "Current default version: $DEFAULT" >&2

COMMIT="$(git rev-parse HEAD)"
if [[ "$DEFAULT" == "$COMMIT" ]]; then
  echo "The website is already serving the latest version, skipping deployment..."
  exit 0
else
  echo "Deploying Kythe website version $COMMIT" >&2
fi

bazel build //kythe/web/site
rsync -Lr --delete "$(bazel info workspace)/bazel-bin/kythe/web/site/_site/" "$PWD/site/"
gcloud app deploy --no-promote --version "$COMMIT" app.yaml

echo >&2
echo "Deployment location: https://$COMMIT-dot-kythe-repo.appspot.com" >&2


gcloud app versions migrate "$COMMIT" --service "$MODULE"
gcloud app versions delete "$DEFAULT" --service "$MODULE"
