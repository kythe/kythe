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
# This script is expected to be inlined into another script via source.
#
# It expects the following variables to be set:
#   OUT_DIR   - temporary output directory for the test
#   TEST_JSON - json dump of Kythe entries for test
#
# It will set or clobber the following variables:
#   COUNTDOWN
#   LISTEN_TO - the host/port passed to http_server
#   server_addr
#   SERVER_PID - the PID for http_server
#   KYTHE_ENTRYSTREAM   - binary path to the entrystream tool
#   KYTHE_HTTP_SERVER   - binary path to the http_server tool
#   KYTHE_WRITE_ENTRIES - binary path to the write_entries tool
#   KYTHE_WRITE_TABLES  - binary path to the write_tables tool
#   XREFS_URI - the URI of the running xrefs service
#
# It will destroy the following directories:
#   "${OUT_DIR}/gs"
#   "${OUT_DIR}/tables"
#
# It depends on the following Kythe tool targets:
#   "//kythe/go/serving/tools:http_server",
#   "//kythe/go/platform/tools:entrystream",
#   "//kythe/go/storage/tools:write_tables",
#   "//kythe/go/storage/tools:write_entries",

KYTHE_WRITE_TABLES="${PWD}/campfire-out/bin/kythe/go/serving/tools/write_tables"
KYTHE_WRITE_ENTRIES="${PWD}/campfire-out/bin/kythe/go/storage/tools/write_entries"
KYTHE_ENTRYSTREAM="${PWD}/campfire-out/bin/kythe/go/platform/tools/entrystream"
KYTHE_HTTP_SERVER="${PWD}/campfire-out/bin/kythe/go/serving/tools/http_server"

if grep ':/docker' /proc/1/cgroup ; then
# lsof doesn't work inside docker because of AppArmor.
# Until that's fixed, we'll pick our own port.
  LISTEN_TO="localhost:31338"
  server_addr() {
    echo "localhost:31338"
  }
else
  LISTEN_TO="localhost:0"
  server_addr() {
    lsof -a -p "$1" -i -s TCP:LISTEN 2>/dev/null | grep -ohw "localhost:[0-9]*"
  }
fi

rm -rf -- "${OUT_DIR:?no output directory for test}/gs"
rm -rf -- "${OUT_DIR:?no output directory for test}/tables"
mkdir -p "${OUT_DIR}/gs"
mkdir -p "${OUT_DIR}/tables"

cat "${TEST_JSON:?no test json for test}" \
    | "${KYTHE_ENTRYSTREAM}" -read_json=true  \
    | "${KYTHE_WRITE_ENTRIES}" -graphstore "${OUT_DIR}/gs" 2>/dev/null
"${KYTHE_WRITE_TABLES}" -graphstore "${OUT_DIR}/gs" -out="${OUT_DIR}/tables" 2>/dev/null
# TODO(zarko): test against GRPC server implementation
"${KYTHE_HTTP_SERVER}" -serving_table "${OUT_DIR}/tables" -listen="${LISTEN_TO}" 2>/dev/null &
SERVER_PID=$!
trap 'kill $SERVER_PID' EXIT ERR INT

COUNTDOWN=16
while :; do
  sleep 1s
  LISTEN_AT=$(server_addr "$SERVER_PID") \
    && curl -s "$LISTEN_AT" >/dev/null \
    && break
  echo "Waiting for server ($COUNTDOWN seconds remaining)..." >&2
  if [[ $((COUNTDOWN--)) -eq 0 ]]; then
    echo "Aborting (launching server took too long)" >&2
    exit 1
  fi
done

XREFS_URI="http://${LISTEN_AT}"
