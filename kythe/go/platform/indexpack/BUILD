load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexpack",
    srcs = ["indexpack.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/vfs",
        "//kythe/go/platform/vfs/zip",
        "@go_uuid//:uuid",
    ],
)

go_test(
    name = "indexpack_test",
    size = "small",
    srcs = ["indexpack_test.go"],
    library = "indexpack",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)
