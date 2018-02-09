load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(
    default_visibility = ["//kythe:default_visibility"],
    licenses = ["notice"],
)

go_library(
    name = "explore",
    srcs = ["explore.go"],
    deps = [
        "//kythe/proto:explore_go_proto",
        "@go_grpc//:codes",
        "@go_grpc//:status",
        "@go_protobuf//:proto",
    ],
)
