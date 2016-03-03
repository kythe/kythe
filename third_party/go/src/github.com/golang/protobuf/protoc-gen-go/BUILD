package(default_visibility = ["//visibility:public"])

load("//tools:build_rules/go.bzl", "go_binary")

licenses(["notice"])

go_binary(
    name = "protoc-gen-go",
    srcs = [
        "link_grpc.go",
        "main.go",
    ],
    deps = [
        "//third_party/go/src/github.com/golang/protobuf/proto",
        "//third_party/go/src/github.com/golang/protobuf/protoc-gen-go/generator",
        "//third_party/go/src/github.com/golang/protobuf/protoc-gen-go/internal/grpc",
    ],
)
