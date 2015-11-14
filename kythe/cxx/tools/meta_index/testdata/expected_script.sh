#!/bin/bash
: "${KYTHE_GRAPHSTORE:?No KYTHE_GRAPHSTORE specified.}"
KYTHE_GRAPHSTORE="$(realpath ${KYTHE_GRAPHSTORE})"
export KYTHE_VNAMES="actual_vnames.json"
KYTHE_SUBGRAPH="actual_subgraph.entries"
(cd kythe/ts && \
  node build/lib/main.js --skipDefaultLib \
    '--rootDir' \
    '/kythe/ts/testdata/repo' \
    '--' \
    'implicit=typings/node/node.d.ts' \
    '/kythe/ts/testdata/repo/index.js' \
 | /opt/kythe/tools/entrystream --read_json \
 | /opt/kythe/tools/write_entries -graphstore \
    "${KYTHE_GRAPHSTORE}")
(cd kythe/ts && \
  node build/lib/main.js --skipDefaultLib \
    '--rootDir' \
    '/kythe/ts/testdata/repo/node_modules/deptwo' \
    '--' \
    'implicit=typings/node/node.d.ts' \
    '/kythe/ts/testdata/repo/node_modules/deptwo/main.js' \
 | /opt/kythe/tools/entrystream --read_json \
 | /opt/kythe/tools/write_entries -graphstore \
    "${KYTHE_GRAPHSTORE}")
(cd kythe/ts && \
  node build/lib/main.js --skipDefaultLib \
    '--rootDir' \
    '/kythe/ts/testdata/repo/node_modules/depone' \
    '--' \
    'implicit=typings/node/node.d.ts' \
    '/kythe/ts/testdata/repo/node_modules/depone/index.js' \
 | /opt/kythe/tools/entrystream --read_json \
 | /opt/kythe/tools/write_entries -graphstore \
    "${KYTHE_GRAPHSTORE}")

/opt/kythe/tools/write_entries -graphstore "${KYTHE_GRAPHSTORE}" \
    < "${KYTHE_SUBGRAPH}"
if [[ ! -z "${KYTHE_TABLES}" ]]; then
  /opt/kythe/tools/write_tables -graphstore "${KYTHE_GRAPHSTORE}" \
      -out="${KYTHE_TABLES}"
  if [[ ! -z "${KYTHE_LISTEN_AT}" ]]; then
    /opt/kythe/tools/http_server -serving_table "${KYTHE_TABLES}" \
        -public_resources="/opt/kythe/web/ui" \
        -listen="${KYTHE_LISTEN_AT}"
  fi
fi
exit 0
