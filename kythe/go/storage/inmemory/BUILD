load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "inmemory",
    srcs = ["inmemory.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)
