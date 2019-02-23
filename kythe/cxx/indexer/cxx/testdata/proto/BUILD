load(":cc_proto_verifier_test.bzl", "cc_proto_verifier_test")

cc_proto_verifier_test(
    name = "proto_test",
    size = "medium",
    srcs = ["proto.cc"],
    proto_srcs = [
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/proto:testdata.proto",
    ],
)
