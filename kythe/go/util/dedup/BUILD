load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "dedup",
    srcs = ["dedup.go"],
    importpath = "kythe.io/kythe/go/util/dedup",
)

go_test(
    name = "dedup_test",
    size = "small",
    srcs = ["dedup_test.go"],
    library = ":dedup",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
