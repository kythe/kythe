load("//tools:build_rules/proto.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

proto_library(
    name = "extra_actions_base_proto",
    srcs = ["src/main/protobuf/extra_actions_base.proto"],
    cc_api_version = 2,
    go_api_version = 2,
    gofast = 0,  # protoc-gen-gofast fails due the extensions in this proto
    java_api_version = 2,
)

proto_library(
    name = "test_status_proto",
    srcs = ["src/main/protobuf/test_status.proto"],
    go_api_version = 2,
)

sh_binary(
    name = "get_devdir",
    srcs = ["get_devdir.sh"],
)

sh_binary(
    name = "get_sdkroot",
    srcs = ["get_sdkroot.sh"],
)
