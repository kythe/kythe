#!/bin/bash
set -e

if [ -z "${CLANG_TIDY:=$(which clang-tidy)}" ]; then
  echo "Unable to find clang-tidy" 1>&2
  exit 1
fi

$(dirname "${BASH_SOURCE[0]}")/../cpp/generate_compilation_database.sh

"$CLANG_TIDY" -p "$(bazel info execution_root)" "$@"

