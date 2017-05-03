load("@io_bazel_rules_go//go:def.bzl", "go_prefix", "go_library")

def external_go_package(base_pkg=None, name=None, deps=[], exclude_srcs=[]):
  """Macro which combines common aliases and globs for external go packages.

  Defines a `go_library` of all Go sources files, except explicit exclusions
  and test files.

  If `name` is provided, the source files are assumed to live in a
  subdirectory of the package.  Otherwise, the target is given the same
  name as the basename of the package.

  Declares `go_prefix(base_pkg)` if none has been declared.
  """
  prefix_rule = native.existing_rule("go_prefix")
  if prefix_rule != None and base_pkg:
    if prefix_rule["prefix"] != base_pkg:
      fail("base_pkg '%s' conflicts with existing prefix '%s'" %
           (base_pkg, prefix_rule["prefix"]))
    base_pkg = prefix_rule["prefix"]
  elif base_pkg:
    go_prefix(base_pkg)

  exclude_srcs += ["*_test.go"]
  if name:
    srcs = [name + "/*.go", name + "/*.s", name + "/*.S"]
    exclude_srcs = [name + "/" + src for src in exclude_srcs]
  else:
    srcs = ["*.go", "*.s", "*.S"]
    name = "go_default_library"
    native.alias(
        name = base_pkg.split("/")[-1],
        actual = ":go_default_library"
    )

  go_library(
      name = name,
      srcs = native.glob(srcs, exclude=exclude_srcs),
      deps = deps,
  )
