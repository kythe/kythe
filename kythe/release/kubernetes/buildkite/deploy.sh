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
#
# Script to build and deploy the kythe-repo/buildkite-agent on Kubernetes.
# Note: if only the image has changed, kubectl apply will not work by itself
#       and must be followed by:
#       kubectl rollout restart deployment.app/buildkite-agent
cd "$(dirname "$0")"

if [[ "$1" == --cloud ]]; then
  gcloud builds submit .
else
  docker build -t gcr.io/kythe-repo/buildkite-agent .
  docker push gcr.io/kythe-repo/buildkite-agent
fi
kubectl apply -f deployment.yaml
