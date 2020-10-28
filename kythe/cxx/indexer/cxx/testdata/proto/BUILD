load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":cc_proto_verifier_test.bzl", "cc_proto_verifier_test")

cc_proto_verifier_test(
    name = "proto_test",
    size = "medium",
    srcs = ["proto.cc"],
    proto_libs = ["//kythe/testdata/indexers/proto:testdata_proto"],
)

cc_proto_verifier_test(
    name = "proto2_test",
    size = "medium",
    srcs = ["proto2.cc"],
    proto_libs = ["//kythe/testdata/indexers/proto:testdata2_proto"],
)

cc_proto_verifier_test(
    name = "proto3_test",
    size = "medium",
    srcs = ["proto3.cc"],
    proto_libs = [
        "//kythe/testdata/indexers/proto:testdata3_proto",
        "//kythe/testdata/indexers/proto:testdata3a_proto",
        "//kythe/testdata/indexers/proto:testdata3b_proto",
    ],
)

cc_proto_verifier_test(
    name = "proto4_test",
    size = "medium",
    srcs = ["proto4.cc"],
    proto_libs = [
        "//kythe/testdata/indexers/proto:testdata4b_proto",
        "//kythe/testdata/indexers/proto:testdata4a_proto",
        "//kythe/testdata/indexers/proto:testdata4c_proto",
    ],
)

bzl_library(
    name = "cc_proto_verifier_test_bzl",
    srcs = ["cc_proto_verifier_test.bzl"],
)
