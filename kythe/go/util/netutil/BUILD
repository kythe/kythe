load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "netutil",
    srcs = ["netutil.go"],
    importpath = "kythe.io/kythe/go/util/netutil",
    deps = ["//kythe/go/util/log"],
)

go_test(
    name = "netutil_test",
    size = "small",
    srcs = ["netutil_test.go"],
    library = ":netutil",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
