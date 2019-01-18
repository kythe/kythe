load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_proto_verifier_test")

java_proto_verifier_test(
    name = "proto",
    srcs = ["Proto.java"],
    proto_srcs = ["testdata.proto"],
    # TODO(schroederc): test does not work with RBE (https://github.com/bazelbuild/bazel/issues/1025)
    tags = ["manual"],
)
