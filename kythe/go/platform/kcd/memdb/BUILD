load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "memdb",
    srcs = ["memdb.go"],
    deps = [
        "//kythe/go/platform/kcd",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "memdb_test",
    size = "small",
    srcs = ["memdb_test.go"],
    library = "memdb",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/platform/kcd/testutil"],
)
