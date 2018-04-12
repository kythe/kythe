package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")
load("@io_bazel_rules_go//go:def.bzl", "go_prefix", "go_library")

licenses(["notice"])

exports_files(["LICENSE"])

go_prefix("github.com/google/go-cmp/cmp/internal/")

go_library(
    name = "value",
    srcs = glob(
         ["cmp/internal/value/*.go"],
         exclude = ["cmp/internal/value/*_test.go"],
    )
)

go_library(
    name = "diff",
    srcs = glob(
         ["cmp/internal/diff/*.go"],
         exclude = ["cmp/internal/diff/*_test.go"],
    ),
)

go_library(
    name = "function",
    srcs = glob(
         ["cmp/internal/function/*.go"],
         exclude = ["cmp/internal/function/*_test.go"],
    ),
)
