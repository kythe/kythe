#!/bin/bash
# Rebuild the versions.bzl from the latest image and current minimum and maximum
# bazel version.
# This should be run if there is a new RBE image or the configuration changes.
cp "$(dirname "$0")/empty-versions.bzl" "$(dirname "$0")/versions.bzl"
RBE_AUTOCONF_ROOT="$(bazel info workspace)"
export RBE_AUTOCONF_ROOT

VERSIONS=(
  "$(cat "$RBE_AUTOCONF_ROOT/.bazelversion")"
  "$(cat "$RBE_AUTOCONF_ROOT/.bazelminversion")"
)

TARGETS=(
  "@rbe_default//..."
  "@rbe_bazel_minversion//..."
  "@rbe_bazel_maxversion//..."
)

for version in "${VERSIONS[@]}"; do
  for target in "${TARGETS[@]}"; do
    USE_BAZEL_VERSION="$version" bazel --bazelrc=/dev/null query "$target"
  done
done
