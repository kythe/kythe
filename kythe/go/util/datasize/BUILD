load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "datasize",
    srcs = ["datasize.go"],
)

go_test(
    name = "datasize_test",
    size = "small",
    srcs = ["datasize_test.go"],
    library = "datasize",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
