load(":cc_proto_verifier_test.bzl", "cc_proto_verifier_test")

cc_proto_verifier_test(
    name = "proto_test",
    size = "medium",
    srcs = ["proto.cc"],
    proto_lib = "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata_proto",
    proto_srcs = [
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata.proto",
    ],
)

cc_proto_verifier_test(
    name = "proto2_test",
    size = "medium",
    srcs = ["proto2.cc"],
    proto_lib = "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata2_proto",
    proto_srcs = [
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata2.proto",
    ],
)
