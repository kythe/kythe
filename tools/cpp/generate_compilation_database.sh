#!/bin/bash

# Generates a compile_commands.json file at $(bazel info execution_root) for
# your Clang tooling needs.

set -e

bazel build \
  --experimental_action_listener=//kythe/cxx/tools/generate_compile_commands:extract_json \
  --noshow_progress \
  --noshow_loading_progress \
  $(bazel query 'kind(cc_.*, //...) - attr(tags, manual, //...)') > /dev/null

BAZEL_ROOT="$(bazel info execution_root)"
pushd "$BAZEL_ROOT" > /dev/null
echo "[" > compile_commands.json
COUNT=0
find . -name '*.compile_command.json' -print0 | while read -r -d '' fname; do
  if ((COUNT++)); then
    echo ',' >> compile_commands.json
  fi
  sed -e "s|@BAZEL_ROOT@|$BAZEL_ROOT|g" < "$fname" >> compile_commands.json
done
echo "]" >> compile_commands.json
popd > /dev/null
