#!/bin/bash -e
#
# Script to build/run the Docker openjdk-extractor container.
#
# Usage: ./kythe/extractors/openjdk/run.sh output-path
#
# Example: ./kythe/extractors/openjdk/run.sh /tmp/idx/java/

usage() {
  echo "usage: $(basename "$0") output-path" >&2
  exit $1
}

OUTPUT="$1"
if [[ -z "$OUTPUT" ]]; then
  usage 1
fi

mkdir -p "$OUTPUT"
OUTPUT="$(readlink -e "$OUTPUT")"

cd "$(dirname "$0")"/../../..
./campfire package --start_registry=false //kythe/extractors/openjdk

docker run -ti --rm -v $OUTPUT:/idx openjdk-extractor
