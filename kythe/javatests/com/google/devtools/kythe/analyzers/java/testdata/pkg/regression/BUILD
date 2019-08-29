load("@rules_java//java:defs.bzl", "java_library")
load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_verifier_test")
load("//kythe/java/com/google/devtools/kythe/extractors/java/bazel:aspect.bzl", "extract_java")

java_library(
    name = "regression",
    srcs = glob(["**/*.java"]),
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata:__subpackages__"],
)

extract_java(
    name = "regression_kzip",
    compilation = ":regression",
    visibility = ["//visibility:public"],
)

java_verifier_test(
    name = "nullable_test",
    size = "small",
    srcs = ["Nullable.java"],
)
