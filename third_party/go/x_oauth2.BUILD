package(default_visibility = ["@//visibility:public"])

load("@io_bazel_rules_go//go:def.bzl", "go_library")

licenses(["notice"])

exports_files(["LICENSE"])

go_library(
    name = "oauth2",
    srcs = glob(
        ["*.go"],
        exclude = ["*_test.go"],
    ),
    importpath = "golang.org/x/oauth2",
    deps = [
        ":internal",
        "@go_x_net//:context",
    ],
)

go_library(
    name = "internal",
    srcs = glob(
        ["internal/*.go"],
        exclude = ["internal/*_test.go"],
    ),
    importpath = "golang.org/x/oauth2/internal",
    deps = [
        "@go_x_net//:context",
        "@go_x_net//:context/ctxhttp",
    ],
    visibility = ["@//visibility:private"],
)
