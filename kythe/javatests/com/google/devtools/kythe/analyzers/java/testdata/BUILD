load("@rules_java//java:defs.bzl", "java_library")
load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_verifier_test")
load("//kythe/java/com/google/devtools/kythe/extractors/java/bazel:aspect.bzl", "extract_java")

exports_files(
    ["corner_case.kindex"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java:__subpackages__"],
)

java_verifier_test(
    name = "default_package_tests",
    size = "small",
    srcs = ["DefaultPackage.java"],
)

java_library(
    name = "testdata",
    srcs = glob(["*.java"]),
    deps = ["@com_google_code_findbugs_jsr305//jar"],  # needed for pkg/package-info.java
)

extract_java(
    name = "testdata_kzip",
    compilation = ":testdata",
    visibility = ["//visibility:public"],
)
