load("@io_bazel_rules_java//java:defs.bzl", "java_library")
load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_extract_kzip", "java_verifier_test")

exports_files(
    ["corner_case.kindex"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java:__subpackages__"],
)

filegroup(
    name = "testdata_files",
    srcs = glob(["*.java"]),
)

# Useful to perform a manual Kythe extraction/analysis of the Java testdata.
java_library(
    name = "testdata",
    srcs = [":testdata_files"],
    exports = [
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg",
    ],
)

java_verifier_test(
    name = "default_package_tests",
    size = "small",
    srcs = ["DefaultPackage.java"],
)

java_extract_kzip(
    name = "testdata_kzip",
    srcs = [
        ":testdata_files",
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg:pkg_files",
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg/regression:regression_files",
    ],
    visibility = ["//visibility:public"],
    deps = ["@com_google_code_findbugs_jsr305//jar"],  # needed for pkg/package-info.java
)
