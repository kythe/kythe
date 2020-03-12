load("//tools/build_rules/verifier_test:java_verifier_test.bzl", "java_verifier_test")

java_verifier_test(
    name = "jdk_tests",
    size = "small",
    srcs = ["SunSource.java"],
    indexer_opts = ["--override_jdk_corpus=override"],
)
