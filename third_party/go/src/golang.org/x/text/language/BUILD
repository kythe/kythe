package(default_visibility = ["//visibility:public"])

load("//third_party/go:build.bzl", "gopath_package")

licenses(["notice"])

gopath_package(
    exclude_srcs = [
        "gen_*.go",
        "go1_1.go",
        "maketables.go",
    ],
    deps = ["//third_party/go/src/golang.org/x/text/internal/tag"],
)
