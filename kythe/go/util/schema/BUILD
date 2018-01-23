load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "schema",
    srcs = ["schema.go"],
    deps = [
        "//kythe/go/util/schema/facts",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "schema_test",
    size = "small",
    srcs = ["schema_test.go"],
    library = "schema",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/util/schema/nodes",
        "@go_protobuf//:proto",
    ],
)
