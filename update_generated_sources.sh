#!/bin/bash -e
# Runs all Bazel targets that write generated files.
#
# Usage:
#   chronic ./update_generated_sources.sh

cd "$(dirname "$0")"

for target in $(bazel query 'kind("_write_source_file", //...)'); do
  bazel build $target
done
