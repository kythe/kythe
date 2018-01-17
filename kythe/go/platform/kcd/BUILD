load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kcd",
    srcs = ["kcd.go"],
)

go_test(
    name = "kcd_test",
    size = "small",
    srcs = ["kcd_test.go"],
    library = "kcd",
    visibility = ["//visibility:private"],
)
