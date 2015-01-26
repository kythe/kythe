#!/bin/bash -e
#
# Script to extract compilations from a campfire-based repository.
#
# Usage:
#   extract.sh [--docker] [campfire-root] <output-dir>

usage() {
  echo "Usage: $(basename "$0") [--docker] [campfire-root] <output-dir>" >&2
  exit 1
}

CAMPFIRE=./campfire
if [[ "$1" == "--docker" ]]; then
  CAMPFIRE=./campfire-docker
  shift
fi

ROOT=.
case $# in
  1)
    OUTPUT="$1" ;;
  2)
    ROOT="$1"
    OUTPUT="$2" ;;
  *)
    usage ;;
esac

mkdir -p "$OUTPUT"
OUTPUT="$(readlink -e "$OUTPUT")"

cd "$ROOT"
"$CAMPFIRE" extract

for idx in $(find campfire-out/gen -name '*.kindex'); do
  name="$(basename "$idx" .kindex)"
  lang="${name##*.}"
  dir="$OUTPUT/$lang"
  mkdir -p "$dir"
  dest="$dir/$(tr '/' '_' <<<"${idx#campfire-out/gen/}")"
  cp "$idx" "$dest" # TODO(schroederc): put into indexpack instead
done
