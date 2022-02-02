#!/bin/bash
# Rebuild the versions.bzl from the latest image and current minimum and maximum
# bazel version.
# This should be run if there is a new RBE image or the configuration changes.
BAZEL_WORKSPACE="${BUILD_WORKSPACE_DIRECTORY?:Must be invoked via 'bazel run'}"

exec "${RBE_GEN_CONFIG}" \
  --output_src_root="${BAZEL_WORKSPACE}" \
  --output_config_path="${OUTPUT_CONFIG_PATH?:Missing mandatory OUTPUT_CONFIG_PATH}" \
  --bazel_version="$(cat "${BAZEL_WORKSPACE}/.bazelversion")" \
  --toolchain_container=gcr.io/kythe-repo/kythe-builder:latest \
  --exec_os=linux --target_os=linux
