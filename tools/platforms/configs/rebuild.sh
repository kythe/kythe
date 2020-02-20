#!/bin/bash
# Rebuild the versions.bzl from the latest image and current minimum and maximum
# bazel version.
# This should be run if there is a new RBE image or the configuration changes.
cp "$(dirname "$0")/empty-versions.bzl" "$(dirname "$0")/versions.bzl"
RBE_AUTOCONF_ROOT="$(bazel info workspace)"
export RBE_AUTOCONF_ROOT
for config in min max; do
  bazel --bazelrc=/dev/null build "@rbe_bazel_${config}version//..."
done
