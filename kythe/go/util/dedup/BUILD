load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "dedup",
    srcs = ["dedup.go"],
)

go_test(
    name = "dedup_test",
    size = "small",
    srcs = ["dedup_test.go"],
    library = "dedup",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
