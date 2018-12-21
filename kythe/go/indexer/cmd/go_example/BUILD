load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "go_example",
    srcs = ["go_example.go"],
    data = ["@go_sdk//:files"],
    deps = [
        "//kythe/go/extractors/golang",
        "//kythe/go/indexer",
        "//kythe/go/platform/delimited",
        "//kythe/go/platform/kindex",
        "//kythe/go/platform/vfs",
        "//kythe/proto:storage_go_proto",
    ],
)
