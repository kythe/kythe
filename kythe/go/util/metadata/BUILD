load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "metadata",
    srcs = ["metadata.go"],
    deps = [
        "//kythe/go/util/schema/edges",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:protoc-gen-go/descriptor",
    ],
)

go_test(
    name = "metadata_test",
    size = "small",
    srcs = ["metadata_test.go"],
    library = ":metadata",
    deps = [
        "//kythe/go/test/testutil",
        "@go_protobuf//:proto",
    ],
)
