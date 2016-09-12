load(
    "@io_bazel_rules_go//go:def.bzl",
    g_go_binary = "go_binary",
    g_go_library = "go_library",
    g_go_test = "go_test",
)

# Re-export the upstream rules.
go_library = g_go_library

go_binary = g_go_binary

go_test = g_go_test

def go_package(name=None,
               deps=[], test_deps=[], test_args=[], test_data=[],
               tests=True, exclude_srcs=[],
               visibility=None):
  """Macro for easily defining a Go package and test in the current directory.

  This is a shortcut rule for single-package directories.

  Args:
    name: The name of the package.
      If omitted, defaults to the directory name.
    deps: Dependencies for the library.
    test_deps: Depedencies for the test.
    test_data: Data dependencies for the test.
    tests: If true (default), whether to include the go_test target.
    exclude_srcs: List of globs to exclude from the library sources.
    visibility: If set, the visibility to use for the library.
  """
  if not name:
    name = "go_default_library"
    alias = PACKAGE_NAME.split("/")[-1]
    native.alias(name = alias, actual = name)
  else:
    alias = name

  exclude = []
  for src in exclude_srcs:
    exclude += [src]

  lib_srcs, test_srcs = [], []
  for src in native.glob(["*.go"], exclude=exclude, exclude_directories=1):
    if src.endswith("_test.go"):
      test_srcs += [src]
    else:
      lib_srcs += [src]

  go_library(
    name = name,
    srcs = lib_srcs,
    deps = deps,
    visibility = visibility,
  )

  if tests and test_srcs:
    go_test(
      name = alias + "_test",
      srcs = test_srcs,
      library = ":" + name,
      deps = test_deps,
      args = test_args,
      data = test_data,
      visibility = ["//visibility:private"],
    )
