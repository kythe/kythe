#!/bin/bash
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
#
# Entrypoint for the kythe-repo/buildbot AppEngine module.

echo "Upgrading master"
buildbot upgrade-master /buildbot/master
# buildbot cleanupdb /buildbot/master
echo "Starting master"
buildbot start /buildbot/master || { echo "Waiting for master..."; sleep 10s; }
echo "Starting worker"
buildbot-worker start /buildbot/worker

tail -f /buildbot/{master,worker}/twistd.log
