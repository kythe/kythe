package(default_visibility = ["//visibility:public"])

load("@//tools:build_rules/go.bzl", "go_binary")
load("@//third_party:go/build.bzl", "external_go_package")

licenses(["notice"])

go_binary(
    name = "protoc-gen-gofast",
    srcs = ["protoc-gen-gofast/main.go"],
    deps = [
        ":vanity",
        ":vanity/command",
    ],
)

external_go_package(
    name = "gogoproto",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":proto",
        ":protoc-gen-gogo/descriptor",
    ],
)

external_go_package(
    name = "protoc-gen-gogo/generator",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":proto",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/plugin",
    ],
)

external_go_package(
    name = "protoc-gen-gogo/descriptor",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":proto",
    ],
)

external_go_package(
    name = "protoc-gen-gogo/plugin",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":proto",
        ":protoc-gen-gogo/descriptor",
    ],
)

external_go_package(
    name = "plugin/enumstringer",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/union",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/description",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/embedcheck",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/defaultcheck",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/size",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":proto",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
        ":vanity",
    ],
)

external_go_package(
    name = "plugin/oneofcheck",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/testgen",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/populate",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":proto",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
        ":vanity",
    ],
)

external_go_package(
    name = "plugin/grpc",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/unmarshal",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":proto",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/face",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/stringer",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/compare",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":proto",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
        ":vanity",
    ],
)

external_go_package(
    name = "plugin/equal",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":proto",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
        ":vanity",
    ],
)

external_go_package(
    name = "plugin/gostring",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":plugin/testgen",
        ":protoc-gen-gogo/generator",
    ],
)

external_go_package(
    name = "plugin/marshalto",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":proto",
        ":protoc-gen-gogo/descriptor",
        ":protoc-gen-gogo/generator",
        ":vanity",
    ],
)

external_go_package(
    name = "vanity",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":gogoproto",
        ":proto",
        ":protoc-gen-gogo/descriptor",
    ],
)

external_go_package(
    name = "vanity/command",
    base_pkg = "github.com/gogo/protobuf",
    deps = [
        ":plugin/compare",
        ":plugin/defaultcheck",
        ":plugin/description",
        ":plugin/embedcheck",
        ":plugin/enumstringer",
        ":plugin/equal",
        ":plugin/face",
        ":plugin/gostring",
        ":plugin/grpc",
        ":plugin/marshalto",
        ":plugin/oneofcheck",
        ":plugin/populate",
        ":plugin/size",
        ":plugin/stringer",
        ":plugin/testgen",
        ":plugin/union",
        ":plugin/unmarshal",
        ":proto",
        ":protoc-gen-gogo/generator",
        ":protoc-gen-gogo/plugin",
    ],
)

external_go_package(
    name = "proto",
    base_pkg = "github.com/gogo/protobuf",
    exclude_srcs = ["pointer_reflect.go"],
)
