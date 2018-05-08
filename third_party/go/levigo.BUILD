package(default_visibility = ["@//visibility:public"])

load("@io_bazel_rules_go//go:def.bzl", "go_library")

licenses(["notice"])

exports_files(["LICENSE"])

alias(
    name = "levigo",
    actual = "go_default_library",
)

go_library(
    name = "go_default_library",
    srcs = glob(
        ["*.go"],
        exclude = [
            "*_test.go",
            "doc.go",
        ],
    ),
    cdeps = ["@//third_party/leveldb"],
    cgo = True,
    importpath = "github.com/jmhodges/levigo",
)
