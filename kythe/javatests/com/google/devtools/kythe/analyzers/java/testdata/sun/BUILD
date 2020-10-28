load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_verifier_test")
load("//kythe/java/com/google/devtools/kythe/extractors/java/bazel:aspect.bzl", "extract_java")

java_library(
    name = "jdk",
    srcs = ["SunSource.java"],
)

extract_java(
    name = "jdk_kzip",
    compilation = ":jdk",
    visibility = ["//visibility:public"],
)

java_verifier_test(
    name = "jdk_tests",
    size = "small",
    compilation = ":jdk",
    indexer_opts = ["--override_jdk_corpus=override"],
)
