load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "pathmap",
    srcs = ["mapper.go"],
)

go_test(
    name = "pathmap_test",
    size = "small",
    srcs = ["mapper_test.go"],
    library = "pathmap",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
