load("//tools:build_rules/go.bzl", "go_release_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_release_binary(
    name = "go_indexer",
    srcs = ["go_indexer.go"],
    deps = [
        "//kythe/go/indexer",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/metadata",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
)
