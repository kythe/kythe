load("@bazel_skylib//rules:copy_file.bzl", "copy_file")
load("@rules_proto//proto:defs.bzl", "proto_library")

package(default_visibility = ["//visibility:public"])

exports_files(glob(["*.proto"]))

proto_library(
    name = "testdata_proto",
    srcs = ["testdata.proto"],
)

proto_library(
    name = "testdata2_proto",
    srcs = ["testdata2.proto"],
)

proto_library(
    name = "testdata3a_proto",
    srcs = ["testdata3a.proto"],
)

proto_library(
    name = "testdata3b_proto",
    srcs = ["testdata3b.proto"],
)

proto_library(
    name = "testdata3_proto",
    srcs = ["testdata3.proto"],
    deps = [
        ":testdata3a_proto",
        ":testdata3b_proto",
    ],
)

proto_library(
    name = "testdata4a_proto",
    srcs = ["testdata4a.proto"],
    deps = [":testdata4c_proto"],
)

proto_library(
    name = "testdata4b_proto",
    srcs = ["testdata4b.proto"],
    deps = [":testdata4c_proto"],
)

proto_library(
    name = "testdata4c_proto",
    srcs = ["testdata4c.proto"],
)

copy_file(
    name = "testdata5_generated",
    src = "testdata5.proto",
    out = "gen/testdata5.gen.proto",
    visibility = ["//visibility:private"],
)

proto_library(
    name = "testdata5_proto",
    srcs = [":testdata5_generated"],
)

proto_library(
    name = "enum_proto",
    srcs = ["enum.proto"],
)
