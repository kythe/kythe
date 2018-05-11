package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")
load("@io_bazel_rules_go//go:def.bzl", "go_binary")

licenses(["notice"])

exports_files(["LICENSE"])

external_go_package(
    name = "pkg/lsp",
    base_pkg = "github.com/sourcegraph/go-langserver",
)
