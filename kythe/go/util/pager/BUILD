load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "pager",
    srcs = ["pager.go"],
    deps = ["//kythe/go/util/sortutil"],
)

go_test(
    name = "pager_test",
    size = "small",
    srcs = ["pager_test.go"],
    library = "pager",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
