load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "proxy",
    srcs = ["proxy.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/gsutil",
        "//kythe/proto:storage_go_proto",
    ],
)

go_test(
    name = "proxy_test",
    size = "small",
    srcs = ["proxy_test.go"],
    library = "proxy",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)
