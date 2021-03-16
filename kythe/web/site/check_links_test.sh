#!/bin/bash
set -e
JEKYLL_BIN="$(realpath "$JEKYLL_BIN")"
cd "kythe/web/site"
"${JEKYLL_BIN}" serve --skip-initial-build --no-watch &
server_pid=$!
trap 'kill -INT "$server_pid"' EXIT ERR INT

for ((i=0; i < 10; i++)); do
  if curl -s localhost:4000 >/dev/null; then
    break
  fi
  echo 'Waiting for server...' >&2
  sleep 1s
done

set -o pipefail
wget --spider -nv -e robots=off -r -p http://localhost:4000 2>&1 | \
  grep -v -e 'unlink: ' -e ' URL:'

# If grep finds a match, exit with a failure.
if [[ "${PIPESTATUS[1]}" -ne 1 ]]; then
  exit 1
fi
