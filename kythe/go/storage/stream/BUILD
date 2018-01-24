load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "stream",
    srcs = ["stream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:jsonpb",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "stream_test",
    size = "small",
    srcs = ["stream_test.go"],
    library = "stream",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/test/testutil"],
)
