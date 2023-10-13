load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "bin",
    srcs = ["main.go"],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/go/languageserver",
        "//kythe/go/services/xrefs",
        "@com_github_sourcegraph_jsonrpc2//:jsonrpc2",
    ],
)
