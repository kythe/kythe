#!/bin/bash
set -e

if [ -z "${CLANG_TIDY:=$(which clang-tidy)}" ]; then
  echo "Unable to find clang-tidy" 1>&2
  exit 1
fi

echo -n "Rebuilding compilations database..."
bazel build \
  --experimental_action_listener=//kythe/cxx/tools/generate_compile_commands:extract_json \
  $(bazel query 'kind(cc_.*, //...)') > /dev/null 2>&1
echo "Done"

pushd $(bazel info execution_root) > /dev/null
echo "[" > compile_commands.json
find . -name '*.compile_command.json' -exec bash -c 'cat {} && echo ,' \; >> compile_commands.json
echo "]" >> compile_commands.json
popd > /dev/null

"$CLANG_TIDY" -p "$(bazel info execution_root)" "$@"
