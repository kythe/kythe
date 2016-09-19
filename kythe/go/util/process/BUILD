load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "process",
    srcs = ["async.go"],
)

go_test(
    name = "process_test",
    srcs = ["process_test.go"],
    library = "process",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
