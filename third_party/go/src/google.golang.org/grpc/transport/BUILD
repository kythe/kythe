package(default_visibility = ["//visibility:public"])

load("//third_party/go:build.bzl", "gopath_package")

licenses(["notice"])

gopath_package(
    deps = [
        "@go_x_net//:context",
        "@go_x_net//:http2",
        "@go_x_net//:http2/hpack",
        "@go_x_net//:trace",
        "//third_party/go/src/google.golang.org/grpc/codes",
        "//third_party/go/src/google.golang.org/grpc/credentials",
        "//third_party/go/src/google.golang.org/grpc/grpclog",
        "//third_party/go/src/google.golang.org/grpc/metadata",
    ],
)
