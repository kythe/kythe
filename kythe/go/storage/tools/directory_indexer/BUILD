load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "directory_indexer",
    srcs = ["directory_indexer.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/vfs",
        "//kythe/go/storage/vnameutil",
        "//kythe/go/util/flagutil",
        "//kythe/proto:storage_proto_go",
        "@go_x_net//:context",
    ],
)
