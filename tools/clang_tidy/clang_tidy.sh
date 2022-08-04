#!/bin/bash
set -e

if [ -z "${CLANG_TIDY:=$(command -v clang-tidy)}" ]; then
  echo "Unable to find clang-tidy" 1>&2
  exit 1
fi

(cd "$(dirname "${BASH_SOURCE[0]}")"; bazel run //:refresh_compile_commands)


"$CLANG_TIDY" "$@"
