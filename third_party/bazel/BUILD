load("@io_bazel_rules_proto//proto:defs.bzl", "proto_library")
load("@io_bazel_rules_java//java:defs.bzl", "java_proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

proto_library(
    name = "extra_actions_base_proto",
    srcs = ["src/main/protobuf/extra_actions_base.proto"],
)

cc_proto_library(
    name = "extra_actions_base_cc_proto",
    deps = [":extra_actions_base_proto"],
)

go_proto_library(
    name = "extra_actions_base_go_proto",
    importpath = "kythe.io/third_party/bazel/extra_actions_base_go_proto",
    proto = ":extra_actions_base_proto",
)

java_proto_library(
    name = "extra_actions_base_java_proto",
    deps = [":extra_actions_base_proto"],
)

proto_library(
    name = "test_status_proto",
    srcs = ["src/main/protobuf/test_status.proto"],
)

cc_proto_library(
    name = "test_status_cc_proto",
    deps = [":test_status_proto"],
)

go_proto_library(
    name = "test_status_go_proto",
    importpath = "kythe.io/third_party/bazel/test_status_go_proto",
    proto = ":test_status_proto",
)

java_proto_library(
    name = "test_status_javaproto",
    deps = [":test_status_proto"],
)

sh_binary(
    name = "get_devdir",
    srcs = ["get_devdir.sh"],
)

sh_binary(
    name = "get_sdkroot",
    srcs = ["get_sdkroot.sh"],
)
