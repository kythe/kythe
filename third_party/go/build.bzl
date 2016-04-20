load("@//tools:build_rules/go.bzl", "go_package")

_gopath_src = "third_party/go/src/"

# Simple wrapper around go_package for third_party/go libraries.
def gopath_package(deps=[], visibility=None, exclude_srcs=[], tests=None):
  pkg = PACKAGE_NAME[len(_gopath_src):].split('/')
  go_package(
    name = pkg[-1],
    package = '/'.join(pkg),
    exclude_srcs = exclude_srcs,
    deps = deps,
    tests = tests,
    visibility = visibility,
  )

# Simple wrapper around go_package for Go packages in external repositories.
def external_go_package(base_pkg, name=None, deps=[], exclude_srcs=[]):
  if name:
    srcs = name
    full_pkg = base_pkg + "/" + name
  else:
    name = base_pkg.split('/')[-1]
    srcs = ""
    full_pkg = base_pkg
  go_package(
      name = name,
      srcs = srcs,
      package = full_pkg,
      exclude_srcs = exclude_srcs,
      deps = deps,
      tests = 0,
  )
