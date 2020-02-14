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
    name = "testdata3b_proto",
    srcs = ["testdata3b.proto"],
    visibility = ["//visibility:public"],
)

proto_library(
    name = "testdata3_proto",
    srcs = ["testdata3.proto"],
    visibility = ["//visibility:public"],
    deps = [
        ":testdata3a_proto",
        ":testdata3b_proto",
    ],
)

proto_library(
    name = "testdata4a_proto",
    srcs = ["testdata4a.proto"],
    visibility = ["//visibility:public"],
    deps = [":testdata4c_proto"],
)

proto_library(
    name = "testdata4b_proto",
    srcs = ["testdata4b.proto"],
    visibility = ["//visibility:public"],
    deps = [":testdata4c_proto"],
)

proto_library(
    name = "testdata4c_proto",
    srcs = ["testdata4c.proto"],
    visibility = ["//visibility:public"],
)
