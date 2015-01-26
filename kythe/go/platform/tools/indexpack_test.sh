#!/bin/bash -e
#
# This script tests the indexpack binary. It requires the jq command (≥ 1.4).

indexpack=campfire-out/bin/kythe/go/platform/tools/indexpack
viewindex=campfire-out/bin/kythe/go/platform/tools/viewindex
test_kindex=kythe/testdata/test.kindex

kindex_contents() {
  $viewindex --files "$1" | jq -c -S .
}

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT ERR INT

$indexpack --quiet --to_archive "$tmp/archive" $test_kindex
$indexpack --quiet --from_archive "$tmp/archive" "$tmp/idx"

kindex_file="$(find "$tmp/idx" -name "*.kindex")"

result="$(kindex_contents "$kindex_file")"
expected="$(kindex_contents $test_kindex)"

if [[ ! "$result" == "$expected" ]]; then
  echo "ERROR: expected $expected; received $result" >&2
  exit 1
fi
