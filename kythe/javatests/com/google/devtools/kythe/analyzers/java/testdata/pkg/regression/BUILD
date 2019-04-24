load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_verifier_test")

filegroup(
    name = "regression_files",
    srcs = glob(["**/*.java"]),
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata:__subpackages__"],
)

java_library(
    name = "regression",
    srcs = [":regression_files"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata:__subpackages__"],
)

java_verifier_test(
    name = "nullable_test",
    size = "small",
    srcs = ["Nullable.java"],
)
