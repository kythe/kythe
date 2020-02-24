load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_proto_verifier_test")

java_proto_verifier_test(
    name = "proto",
    srcs = ["Proto.java"],
    proto_srcs = ["//kythe/testdata/indexers/proto:testdata.proto"],
    verifier_opts = [
        "--ignore_dups",
        "--check_for_singletons=false",
    ],
)

java_proto_verifier_test(
    name = "proto2",
    srcs = ["Proto2.java"],
    proto_srcs = ["//kythe/testdata/indexers/proto:testdata2.proto"],
)
