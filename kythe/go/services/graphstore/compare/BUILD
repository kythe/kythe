load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "compare",
    srcs = ["compare.go"],
    deps = ["//kythe/proto:storage_proto_go"],
)

go_test(
    name = "compare_test",
    size = "small",
    srcs = ["compare_test.go"],
    library = "compare",
    visibility = ["//visibility:private"],
    deps = ["//kythe/proto:storage_proto_go"],
)
