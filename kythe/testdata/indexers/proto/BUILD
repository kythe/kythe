load("@rules_proto//proto:defs.bzl", "proto_library")

exports_files(glob(["*.proto"]))

proto_library(
    name = "testdata_proto",
    srcs = ["testdata.proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "testdata2_proto",
    srcs = ["testdata2.proto"],
    visibility = ["//visibility:public"],
)
