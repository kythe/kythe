load("//tools/build_rules/verifier_test:verifier_test.bzl", "java_verifier_test")

exports_files(
    ["corner_case.kindex"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java:__subpackages__"],
)

# Useful to perform a manual Kythe extraction/analysis of the Java testdata.
java_library(
    name = "testdata",
    srcs = glob(["*.java"]),
    exports = [
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg",
    ],
)

java_verifier_test(
    name = "default_package_tests",
    size = "small",
    srcs = ["DefaultPackage.java"],
)
