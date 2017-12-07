load("//tools:build_rules/go.bzl", "go_release_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_release_binary(
    name = "directory_indexer",
    srcs = ["directory_indexer.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:storage_proto_go",
    ],
)
