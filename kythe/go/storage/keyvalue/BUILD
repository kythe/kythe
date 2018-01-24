load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "keyvalue",
    srcs = ["keyvalue.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/util/datasize",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "keyvalue_test",
    size = "small",
    srcs = ["keyvalue_test.go"],
    library = "keyvalue",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)
