load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "edges",
    srcs = ["edges.go"],
    deps = ["//kythe/go/util/schema"],
)

go_test(
    name = "edges_test",
    size = "small",
    srcs = ["edges_test.go"],
    library = "edges",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
