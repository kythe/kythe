load("@rules_proto//proto:defs.bzl", "proto_library")

exports_files(["BUILD"])

proto_library(
    name = "package_load_metrics_proto",
    srcs = ["package_load_metrics.proto"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_protobuf//:duration_proto"],
)
