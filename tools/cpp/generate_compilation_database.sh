#!/bin/bash

# Generates a compile_commands.json file at $(bazel info execution_root) for
# the given file path.

set -e

FILENAME=${1:?Missing required source path}
bazel build \
  --experimental_action_listener=//kythe/cxx/tools/generate_compile_commands:extract_json \
  --output_groups=compilation_outputs \
  --compile_one_dependency \
  "$FILENAME" > /dev/null

BAZEL_ROOT="$(bazel info execution_root)"
pushd "$BAZEL_ROOT" > /dev/null
find . -name '*.compile_command.json' -print0 | while read -r -d '' fname; do
  sed -e "s|@BAZEL_ROOT@|$BAZEL_ROOT|g" < "$fname" >> compile_commands.json
  echo "" >> compile_commands.json
done
# Decompose, insert and keep the most recent entry for a given file, then
# recombine.
sed 's/\(^[[]\)\|\([],]$\)//;/^$/d;' < compile_commands.json \
  | tac | sort -u -t, -k1,1 \
  | sed '1s/^./[\0/;s/}$/},/;$s/,$/]/' > compile_commands.json.tmp
mv compile_commands.json{.tmp,}
popd > /dev/null
