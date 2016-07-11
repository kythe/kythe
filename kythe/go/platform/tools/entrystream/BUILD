load("//tools:build_rules/go.bzl", "go_binary")

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
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
