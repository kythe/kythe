#! /bin/bash

# This file goes into the //tools/bazel of the top-level repository.

readonly _output_user_root="${HOME}/.cache/bazel/_bazel_${USER}"
readonly _nix_install="${_output_user_root}/nix_install"
readonly _scripts_dir="${_nix_install}/scripts_dir"

export BAZEL_REAL
export NIX_PORTABLE_BINARY="${_scripts_dir}/nix-portable"

"${_scripts_dir}/bazel_wrapper" "${@}"

