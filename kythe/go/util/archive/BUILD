load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "archive",
    srcs = ["reader.go"],
)

go_test(
    name = "archive_test",
    size = "small",
    srcs = ["reader_test.go"],
    library = "archive",
    visibility = ["//visibility:private"],
)
