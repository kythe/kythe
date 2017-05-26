load("//tools:build_rules/go.bzl", "go_package_library", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "vnameutil",
    srcs = [
        "order.go",
        "rewrite.go",
    ],
    deps = ["//kythe/proto:storage_proto_go"],
)

go_test(
    name = "rewrite_test",
    size = "small",
    srcs = ["rewrite_test.go"],
    library = "vnameutil",
    deps = [
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "order_test",
    size = "small",
    srcs = ["order_test.go"],
    library = "vnameutil",
    deps = ["//kythe/proto:storage_proto_go"],
)

# A driver program for testing vname rewrite rule sets.
go_binary(
    name = "test_vname_rules",
    testonly = 1,
    srcs = ["test/test_vname_rules.go"],
    deps = [
        ":vnameutil",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
