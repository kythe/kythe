load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "ptypes",
    srcs = ["ptypes.go"],
    deps = [
        "//kythe/proto:any_go_proto",
        "@go_protobuf//:proto",
        "@go_protobuf//:ptypes",
    ],
)

go_test(
    name = "ptypes_test",
    size = "small",
    srcs = ["ptypes_test.go"],
    library = "ptypes",
    visibility = ["//visibility:private"],
    deps = ["//kythe/proto:any_go_proto"],
)
