load(
    "@io_bazel_rules_go//go:def.bzl",
    "go_library",
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
