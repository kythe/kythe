# gazelle:proto file
# gazelle:go_generate_proto false
# Ignore this file, but generate build files for descendants.
# gazelle:ignore
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_java//java:defs.bzl", "java_proto_library")
load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")
load("@aspect_bazel_lib//lib:write_source_files.bzl", "write_source_file")
load("@aspect_bazel_lib//lib:copy_to_directory.bzl", "copy_to_directory")
load("//kythe/proto:go.bzl", "go_proto_library")
load("//kythe/proto:genproto.bzl", "update_generated_protos")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # Apache 2.0

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

copy_to_directory(
    name = "proto_srcs",
    srcs = [
        # We need to include the Gazelle-generated BUILD files in the original bundle to avoid test failures.
        "//third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto:BUILD",
        "//third_party/bazel/src/main/java/com/google/devtools/build/lib/packages/metrics:BUILD",
        "//third_party/bazel/src/main/protobuf:BUILD",
        "@io_bazel_files//:proto_srcs",
    ],
    include_external_repositories = ["io_bazel_files"],
    replace_prefixes = {
        "src/": "",
        "third_party/bazel/": "",
    },
    visibility = ["//visibility:private"],
)

write_source_file(
    name = "proto_srcs_sync",
    in_file = ":proto_srcs",
    out_file = "src",
    tags = ["local"],  # Fails spuriously remotely.
    visibility = ["//visibility:private"],
)

alias(
    name = "extra_actions_base_proto",
    actual = "//third_party/bazel/src/main/protobuf:extra_actions_base_proto",
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

alias(
    name = "test_status_proto",
    actual = "//third_party/bazel/src/main/protobuf:test_status_proto",
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

alias(
    name = "build_event_stream_proto",
    actual = "//third_party/bazel/src/main/java/com/google/devtools/build/lib/buildeventstream/proto:build_event_stream_proto",
)

cc_proto_library(
    name = "build_event_stream_cc_proto",
    deps = [":build_event_stream_proto"],
)

go_proto_library(
    name = "build_event_stream_go_proto",
    importpath = "kythe.io/third_party/bazel/build_event_stream_go_proto",
    proto = ":build_event_stream_proto",
    deps = [
        ":command_line_go_proto",
        ":failure_details_go_proto",
        ":invocation_policy_go_proto",
        ":package_load_metrics_go_proto",
    ],
)

alias(
    name = "command_line_proto",
    actual = "//third_party/bazel/src/main/protobuf:command_line_proto",
)

go_proto_library(
    name = "command_line_go_proto",
    importpath = "kythe.io/third_party/bazel/command_line_go_proto",
    proto = ":command_line_proto",
    deps = [
        ":option_filters_go_proto",
    ],
)

alias(
    name = "failure_details_proto",
    actual = "//third_party/bazel/src/main/protobuf:failure_details_proto",
)

go_proto_library(
    name = "failure_details_go_proto",
    importpath = "kythe.io/third_party/bazel/failure_details_go_proto",
    proto = ":failure_details_proto",
)

alias(
    name = "option_filters_proto",
    actual = "//third_party/bazel/src/main/protobuf:option_filters_proto",
)

go_proto_library(
    name = "option_filters_go_proto",
    importpath = "kythe.io/third_party/bazel/option_filters_go_proto",
    proto = ":option_filters_proto",
)

alias(
    name = "invocation_policy_proto",
    actual = "//third_party/bazel/src/main/protobuf:invocation_policy_proto",
)

go_proto_library(
    name = "invocation_policy_go_proto",
    importpath = "kythe.io/third_party/bazel/invocation_policy_go_proto",
    proto = ":invocation_policy_proto",
)

alias(
    name = "package_load_metrics_proto",
    actual = "//third_party/bazel/src/main/java/com/google/devtools/build/lib/packages/metrics:package_load_metrics_proto",
)

go_proto_library(
    name = "package_load_metrics_go_proto",
    importpath = "kythe.io/third_party/bazel/package_load_metrics_go_proto",
    proto = ":package_load_metrics_proto",
)

sh_binary(
    name = "get_devdir",
    srcs = ["get_devdir.sh"],
)

sh_binary(
    name = "get_sdkroot",
    srcs = ["get_sdkroot.sh"],
)

update_generated_protos(name = "update")
