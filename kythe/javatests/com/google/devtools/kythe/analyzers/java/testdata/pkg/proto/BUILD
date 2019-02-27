load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_proto_verifier_test")

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

java_proto_verifier_test(
    name = "proto",
    srcs = ["Proto.java"],
    proto_srcs = ["testdata.proto"],
)

java_proto_verifier_test(
    name = "proto2",
    srcs = ["Proto2.java"],
    proto_srcs = ["testdata2.proto"],
)
