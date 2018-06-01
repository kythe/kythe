#!/bin/sh

set -e
readonly output=kythe/go/util/schema/indexdata.go

echo "-- Updating $output from Bazel ... " 1>&2
set -x
bazel build //kythe/go/util/schema:schema_index
cp bazel-genfiles/kythe/go/util/schema/schema_index.go "$output"
chmod 0644 "$output"
