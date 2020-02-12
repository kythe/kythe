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

proto_library(
    name = "testdata3a_proto",
    srcs = ["testdata3a.proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "testdata3_proto",
    srcs = ["testdata3.proto"],
    visibility = ["//visibility:public"],
    deps = [":testdata3a_proto"],
)
