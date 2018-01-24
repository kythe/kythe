load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "driver",
    srcs = ["driver.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/proto:analysis_go_proto",
        "@go_errors//:errors",
    ],
)

go_test(
    name = "driver_test",
    size = "small",
    srcs = ["driver_test.go"],
    library = "driver",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/proto:storage_go_proto",
    ],
)
