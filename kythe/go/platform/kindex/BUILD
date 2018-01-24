load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kindex",
    srcs = ["kindex.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/ptypes",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "kindex_test",
    size = "small",
    srcs = ["kindex_test.go"],
    library = "kindex",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:any_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)
