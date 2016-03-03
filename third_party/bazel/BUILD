load("//tools:build_rules/proto.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

proto_library(
    name = "extra_actions_base_proto",
    src = "src/main/protobuf/extra_actions_base.proto",
    gen_cc = 1,
    gen_go = 1,
    gen_java = 1,
    gofast = 0,  # protoc-gen-gofast fails due the extensions in this proto
)

proto_library(
    name = "test_status_proto",
    src = "src/main/protobuf/test_status.proto",
    gen_go = 1,
)
