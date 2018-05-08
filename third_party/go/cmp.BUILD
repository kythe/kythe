package(default_visibility = ["@//visibility:public"])

load("@io_bazel_rules_go//go:def.bzl", "go_library")

licenses(["notice"])

exports_files(["LICENSE"])

go_library(
    name = "cmp",
    srcs = glob(
        ["cmp/*.go"],
        exclude = ["cmp/*_test.go"],
    ),
    importpath = "github.com/google/go-cmp/cmp",
    deps = [
        "@go_cmp_internal//:diff",
        "@go_cmp_internal//:function",
        "@go_cmp_internal//:value",
    ],
)
