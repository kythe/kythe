load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
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
