load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
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
