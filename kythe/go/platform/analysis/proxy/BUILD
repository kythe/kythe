load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "proxy",
    srcs = ["proxy.go"],
    deps = [
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "proxy_test",
    size = "small",
    srcs = ["proxy_test.go"],
    library = "proxy",
)
