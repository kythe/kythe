load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
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
