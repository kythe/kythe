package(default_visibility = ["@//visibility:public"])

load("@io_kythe//third_party:go/build.bzl", "external_go_package")
load("@io_bazel_rules_go//go:def.bzl", "go_binary")

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
)

external_go_package(
    name = "jsonpb",
    base_pkg = "github.com/golang/protobuf",
    deps = [":proto"],
)

external_go_package(
    name = "ptypes",
    base_pkg = "github.com/golang/protobuf",
    deps = [
        ":proto",
        ":ptypes/any",
        ":ptypes/duration",
        ":ptypes/timestamp",
    ],
)

external_go_package(
    name = "ptypes/any",
    base_pkg = "github.com/golang/protobuf",
    deps = [":proto"],
)

external_go_package(
    name = "ptypes/duration",
    base_pkg = "github.com/golang/protobuf",
    deps = [":proto"],
)

external_go_package(
    name = "ptypes/timestamp",
    base_pkg = "github.com/golang/protobuf",
    deps = [":proto"],
)
