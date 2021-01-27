#!/bin/bash
# Rebuild the versions.bzl from the latest image and current minimum and maximum
# bazel version.
# This should be run if there is a new RBE image or the configuration changes.
HERE="$(dirname "$0")"

RBE_AUTOCONF_ROOT="$(bazel info workspace)"

VERSIONS=(
  "$(cat "$RBE_AUTOCONF_ROOT/.bazelminversion")"
  "$(cat "$RBE_AUTOCONF_ROOT/.bazelversion")"
)

TARGETS=(
  "@rbe_default//..."
)

cp "$HERE/empty-versions.bzl" "$HERE/versions.bzl"
rm -r "$HERE/default_toolchain_config_spec_name"
export RBE_AUTOCONF_ROOT
for version in "${VERSIONS[@]}"; do
  for target in "${TARGETS[@]}"; do
    USE_BAZEL_VERSION="$version" bazel --bazelrc=/dev/null query "$target"
  done
done
