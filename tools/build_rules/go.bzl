load(
    "@io_bazel_rules_go//go:def.bzl",
    "go_library",
    "go_binary",
    g_go_test = "go_test",
)

go_test = g_go_test  # Re-export the go_test rule for simpler loads().

def go_package_library(name, srcs, deps=[], visibility=None):
  """Macro for minimizing differences between go_library rules."""
  if name != PACKAGE_NAME.split("/")[-1]:
    fail("Package name '%s' must be the same as the Blaze package ('%s')" %
         (name, PACKAGE_NAME.split("/")[-1]))

  native.alias(name = name, actual = ":go_default_library")
  go_library(
      name = "go_default_library",
      srcs = srcs,
      deps = deps,
      visibility = visibility,
  )

def go_release_binary(name, srcs, deps=None, visibility=None):
  """Macro for Go binaries destined to be released."""
  go_binary(
      name = name,
      srcs = srcs,
      deps = deps,
      visibility = visibility,
      linkstamp = "kythe.io/kythe/go/util/build",
  )
