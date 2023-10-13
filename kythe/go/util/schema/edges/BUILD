load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "edges",
    srcs = ["edges.go"],
    importpath = "kythe.io/kythe/go/util/schema/edges",
    deps = ["//kythe/go/util/schema"],
)

go_test(
    name = "edges_test",
    size = "small",
    srcs = ["edges_test.go"],
    library = ":edges",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
