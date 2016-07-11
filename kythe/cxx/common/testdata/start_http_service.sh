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
#   KYTHE_BIN    - where the Kythe binaries can be found
#   OUT_DIR      - temporary output directory for the test
#   TEST_ENTRIES - Kythe entries for test (mutually exclusive with TEST_JSON)
#   TEST_JSON    - json dump of Kythe entries for test
#
# It will set or clobber the following variables:
#   COUNTDOWN
#   LISTEN_AT - the server's hostname:port
#   KYTHE_ENTRYSTREAM   - binary path to the entrystream tool
#   KYTHE_HTTP_SERVER   - binary path to the http_server tool
#   KYTHE_WRITE_ENTRIES - binary path to the write_entries tool
#   KYTHE_WRITE_TABLES  - binary path to the write_tables tool
#   PORT_FILE - the name of a file that contains the listening port
#   XREFS_URI - the URI of the running xrefs service
#
# It will destroy the following directories:
#   "${OUT_DIR}/gs"
#   "${OUT_DIR}/tables"
#
# It depends on the following Kythe tool targets:
#   "//kythe/go/platform/tools:entrystream",
#   "//kythe/go/storage/tools:write_tables",
#   "//kythe/go/storage/tools:write_entries",
#   "//kythe/go/test/tools:http_server",

KYTHE_WRITE_TABLES="kythe/go/serving/tools/write_tables/write_tables"
KYTHE_WRITE_ENTRIES="kythe/go/storage/tools/write_entries/write_entries"
KYTHE_ENTRYSTREAM="kythe/go/platform/tools/entrystream/entrystream"
KYTHE_HTTP_SERVER="kythe/go/test/tools/http_server"
PORT_FILE="${OUT_DIR:?no output directory for test}/service_port"

ENTRYSTREAM_ARGS=
if [[ -z "$TEST_ENTRIES" ]]; then
  TEST_ENTRIES="$TEST_JSON"
  ENTRYSTREAM_ARGS=-read_json=true
fi
CAT=cat
if [[ "$TEST_ENTRIES" == *.gz ]]; then
  CAT="gunzip -c"
fi

rm -rf -- "${OUT_DIR:?no output directory for test}/gs"
rm -rf -- "${OUT_DIR:?no output directory for test}/tables"
rm -f -- "${PORT_FILE}"
mkdir -p "${OUT_DIR}/gs"
mkdir -p "${OUT_DIR}/tables"
$CAT "${TEST_ENTRIES:?no test entries for test}" | \
  "${KYTHE_ENTRYSTREAM}" $ENTRYSTREAM_ARGS \
  | "${KYTHE_WRITE_ENTRIES}" -graphstore "${OUT_DIR}/gs" 2>/dev/null
"${KYTHE_WRITE_TABLES}" --graphstore "${OUT_DIR}/gs" --out "${OUT_DIR}/tables"
# TODO(zarko): test against GRPC server implementation
COUNTDOWN=16
"${KYTHE_HTTP_SERVER}" -serving_table "${OUT_DIR}/tables" \
    -port_file="${PORT_FILE}" >&2 &
while :; do
  if [[ -e "${PORT_FILE}" && "$(cat ${PORT_FILE} | wc -l)" -eq 1 ]]; then
    LISTEN_AT="localhost:$(< ${PORT_FILE})"
    trap 'curl -s "$LISTEN_AT/quitquitquit" || true' EXIT ERR INT
    break
  fi
  if [[ $((COUNTDOWN--)) -eq 0 ]]; then
    echo "Aborting (launching server took too long)" >&2
    exit 1
  fi
  sleep 1s
done
COUNTDOWN=16
while :; do
  curl -sf "$LISTEN_AT/alive" >/dev/null && break
  echo "Waiting for server ($COUNTDOWN seconds remaining)..." >&2
  sleep 1s
  if [[ $((COUNTDOWN--)) -eq 0 ]]; then
    echo "Aborting (launching server took too long)" >&2
    exit 1
  fi
done

XREFS_URI="http://${LISTEN_AT}"
