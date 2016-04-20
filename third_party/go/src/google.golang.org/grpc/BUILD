package(default_visibility = ["//visibility:public"])

load("//third_party/go:build.bzl", "gopath_package")

licenses(["notice"])

exports_files(["LICENSE"])

gopath_package(
    deps = [
        "//third_party/go/src/github.com/golang/protobuf/proto",
        "@go_x_net//:context",
        "@go_x_net//:trace",
        "//third_party/go/src/google.golang.org/grpc/codes",
        "//third_party/go/src/google.golang.org/grpc/credentials",
        "//third_party/go/src/google.golang.org/grpc/grpclog",
        "//third_party/go/src/google.golang.org/grpc/metadata",
        "//third_party/go/src/google.golang.org/grpc/naming",
        "//third_party/go/src/google.golang.org/grpc/transport",
    ],
)
