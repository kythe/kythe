load("@rules_proto//proto:defs.bzl", "proto_library")

exports_files(["BUILD"])

proto_library(
    name = "command_line_proto",
    srcs = ["command_line.proto"],
    visibility = ["//visibility:public"],
    deps = [":option_filters_proto"],
)

proto_library(
    name = "extra_actions_base_proto",
    srcs = ["extra_actions_base.proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "failure_details_proto",
    srcs = ["failure_details.proto"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:descriptor_proto"],
)

proto_library(
    name = "invocation_policy_proto",
    srcs = ["invocation_policy.proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "option_filters_proto",
    srcs = ["option_filters.proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "test_status_proto",
    srcs = ["test_status.proto"],
    visibility = ["//visibility:public"],
)
