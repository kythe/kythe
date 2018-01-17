load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "html",
    srcs = ["html.go"],
    deps = ["@go_x_net//:html"],
)

go_test(
    name = "html_test",
    size = "small",
    srcs = ["html_test.go"],
    library = "html",
    visibility = ["//visibility:private"],
    deps = ["@go_x_net//:html"],
)
