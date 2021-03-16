load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_java//java:defs.bzl", "java_proto_library")
load("@io_bazel_rules_rust//proto:proto.bzl", "rust_proto_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

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

rust_proto_library(
    name = "extra_actions_base_rust_proto",
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

proto_library(
    name = "build_event_stream_proto",
    srcs = ["src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.proto"],
    deps = [
        ":command_line_proto",
        ":invocation_policy_proto",
    ],
)

go_proto_library(
    name = "build_event_stream_go_proto",
    importpath = "kythe.io/third_party/bazel/build_event_stream_go_proto",
    proto = ":build_event_stream_proto",
    deps = [
        ":command_line_go_proto",
        ":invocation_policy_go_proto",
    ],
)

proto_library(
    name = "command_line_proto",
    srcs = ["src/main/protobuf/command_line.proto"],
    deps = [
        ":option_filters_proto",
    ],
)

go_proto_library(
    name = "command_line_go_proto",
    importpath = "kythe.io/third_party/bazel/command_line_go_proto",
    proto = ":command_line_proto",
    deps = [
        ":option_filters_go_proto",
    ],
)

proto_library(
    name = "option_filters_proto",
    srcs = ["src/main/protobuf/option_filters.proto"],
)

go_proto_library(
    name = "option_filters_go_proto",
    importpath = "kythe.io/third_party/bazel/option_filters_go_proto",
    proto = ":option_filters_proto",
)

proto_library(
    name = "invocation_policy_proto",
    srcs = ["src/main/protobuf/invocation_policy.proto"],
)

go_proto_library(
    name = "invocation_policy_go_proto",
    importpath = "kythe.io/third_party/bazel/invocation_policy_go_proto",
    proto = ":invocation_policy_proto",
)

sh_binary(
    name = "get_devdir",
    srcs = ["get_devdir.sh"],
)

sh_binary(
    name = "get_sdkroot",
    srcs = ["get_sdkroot.sh"],
)
