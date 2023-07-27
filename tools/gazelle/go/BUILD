load("//tools:build_rules/shims.bzl", "go_library")

go_library(
    name = "go",
    srcs = ["lang.go"],
    importpath = "kythe.io/tools/gazelle/go",
    visibility = ["//visibility:public"],
    deps = [
        "@bazel_gazelle//config:go_default_library",
        "@bazel_gazelle//language:go_default_library",
        "@bazel_gazelle//language/go:go_default_library",
        "@bazel_gazelle//rule:go_default_library",
    ],
)
