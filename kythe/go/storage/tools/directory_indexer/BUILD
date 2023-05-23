load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "directory_indexer",
    srcs = ["directory_indexer.go"],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/log",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:storage_go_proto",
    ],
)
