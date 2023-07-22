load("@rules_proto//proto:defs.bzl", "proto_library")

exports_files(["BUILD"])

proto_library(
    name = "build_event_stream_proto",
    srcs = ["build_event_stream.proto"],
    visibility = ["//visibility:public"],
    deps = [
        "//third_party/bazel/src/main/java/com/google/devtools/build/lib/packages/metrics:package_load_metrics_proto",
        "//third_party/bazel/src/main/protobuf:command_line_proto",
        "//third_party/bazel/src/main/protobuf:failure_details_proto",
        "//third_party/bazel/src/main/protobuf:invocation_policy_proto",
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
)
