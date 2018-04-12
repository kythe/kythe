package(default_visibility = ["@//visibility:public"])

load("@io_bazel_rules_go//go:def.bzl", "go_prefix", "go_library")

licenses(["notice"])

exports_files(["LICENSE"])

go_prefix("github.com/google/go-cmp/cmp")

go_library(
    name = "cmp",
    srcs =  glob(
         ["cmp/*.go"],
         exclude = ["cmp/*_test.go"],
    ),
    deps = [
         "@go_cmp_internal//:value",
         "@go_cmp_internal//:diff",
         "@go_cmp_internal//:function",
    ],
)
