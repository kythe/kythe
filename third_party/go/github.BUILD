package(default_visibility = ["@//visibility:public"])

load("@io_bazel_rules_go//go:def.bzl", "go_library")

licenses(["notice"])

exports_files(["LICENSE"])

go_library(
    name = "github",
    srcs = glob(
        ["github/*.go"],
        exclude = ["github/*_test.go"],
    ),
    importpath = "github.com/google/go-github/github",
    deps = [
        "@go_querystring//:query",
    ],
)
