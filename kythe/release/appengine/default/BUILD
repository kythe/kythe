load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "server",
    srcs = ["main.go"],
    visibility = ["//visibility:public"],
    deps = ["//kythe/go/util/log"],
)
