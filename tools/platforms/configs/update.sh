#!/bin/bash
# Update versions.bzl from the latest image and current maximum bazel version.
# This should be run if there is a new RBE image or the configuration changes.
RBE_AUTOCONF_ROOT="$(bazel info workspace)"
export RBE_AUTOCONF_ROOT
bazel --bazelrc=/dev/null query "@rbe_default//..."
