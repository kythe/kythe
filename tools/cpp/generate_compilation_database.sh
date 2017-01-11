#!/bin/bash

set -e

echo -n "Rebuilding compilations database..."
bazel build \
  --experimental_action_listener=//kythe/cxx/tools/generate_compile_commands:extract_json \
  $(bazel query 'kind(cc_.*, //...)') > /dev/null 2>&1
echo "Done"

pushd $(bazel info execution_root) > /dev/null
echo "[" > compile_commands.json
find . -name '*.compile_command.json' -exec bash -c 'cat {} && echo ,' \; >> compile_commands.json
sed -i '$s/,$//' compile_commands.json
echo "]" >> compile_commands.json
popd > /dev/null

