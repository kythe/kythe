load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "stream",
    srcs = ["stream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/proto:storage_proto_go",
    ],
)

go_test(
    name = "stream_test",
    srcs = ["stream_test.go"],
    library = "stream",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
