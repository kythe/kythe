package(default_visibility = ["//visibility:public"])

load("//third_party/go:build.bzl", "gopath_package")

licenses(["notice"])

gopath_package(
    exclude_srcs = ["cancelreq_go14.go"],
    deps = ["//third_party/go/src/golang.org/x/net/context"],
)
