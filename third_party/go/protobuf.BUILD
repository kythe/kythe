package(default_visibility = ["@//visibility:public"])

load("@//third_party:go/build.bzl", "external_go_package")
load("@//tools:build_rules/go.bzl", "go_binary")

licenses(["notice"])

exports_files(["LICENSE"])

go_binary(
    name = "protoc-gen-golang",
    srcs = [
        "protoc-gen-go/link_grpc.go",
        "protoc-gen-go/main.go",
    ],
    deps = [
        ":proto",
        ":protoc-gen-go/generator",
        ":protoc-gen-go/grpc",
    ],
)

external_go_package(
    name = "protoc-gen-go/generator",
    base_pkg = "github.com/golang/protobuf",
    deps = [
        ":proto",
        ":protoc-gen-go/descriptor",
        ":protoc-gen-go/plugin",
    ],
)

external_go_package(
    name = "protoc-gen-go/plugin",
    base_pkg = "github.com/golang/protobuf",
    deps = [
        ":proto",
        ":protoc-gen-go/descriptor",
    ],
)

external_go_package(
    name = "protoc-gen-go/descriptor",
    base_pkg = "github.com/golang/protobuf",
    deps = [":proto"],
)

external_go_package(
    name = "protoc-gen-go/grpc",
    base_pkg = "github.com/golang/protobuf",
    deps = [
        ":protoc-gen-go/descriptor",
        ":protoc-gen-go/generator",
    ],
)

external_go_package(
    name = "proto",
    base_pkg = "github.com/golang/protobuf",
    exclude_srcs = ["pointer_reflect.go"],
)

external_go_package(
    name = "jsonpb",
    base_pkg = "github.com/golang/protobuf",
    deps = [":proto"],
)
