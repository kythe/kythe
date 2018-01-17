load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "entrystream",
    srcs = ["entrystream.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/stream",
        "//kythe/go/util/disksort",
        "//kythe/go/util/flagutil",
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)
