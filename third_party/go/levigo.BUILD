package(default_visibility = ["@//visibility:public"])

load("@io_bazel_rules_go//go:def.bzl", "go_prefix", "go_library", "cgo_library")

licenses(["notice"])

exports_files(["LICENSE"])

go_prefix("github.com/jmhodges/levigo")

alias(
    name = "levigo",
    actual = "go_default_library",
)

cgo_library(
    name = "go_default_library",
    srcs = glob(
        ["*.go"],
        exclude = [
            "*_test.go",
            "doc.go",
        ],
    ),
    cdeps = ["@io_kythe//third_party/leveldb"],
)
