load(":cc_proto_verifier_test.bzl", "cc_proto_verifier_test")

cc_proto_verifier_test(
    name = "proto_test",
    size = "medium",
    srcs = ["proto.cc"],
    proto_lib = "//kythe/testdata/indexers/proto:testdata_proto",
    proto_srcs = [
        "//kythe/testdata/indexers/proto:testdata.proto",
    ],
)

cc_proto_verifier_test(
    name = "proto2_test",
    size = "medium",
    srcs = ["proto2.cc"],
    proto_lib = "//kythe/testdata/indexers/proto:testdata2_proto",
    proto_srcs = [
        "//kythe/testdata/indexers/proto:testdata2.proto",
    ],
)

cc_proto_verifier_test(
    name = "proto3_test",
    size = "medium",
    srcs = ["proto3.cc"],
    proto_deps = [
        "//kythe/testdata/indexers/proto:testdata3a_proto",
    ],
    proto_lib = "//kythe/testdata/indexers/proto:testdata3_proto",
    proto_srcs = [
        "//kythe/testdata/indexers/proto:testdata3a.proto",
        "//kythe/testdata/indexers/proto:testdata3.proto",
    ],
)
