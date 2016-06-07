load("//tools:build_rules/kythe.bzl", "java_verifier_test")

exports_files(
    ["corner_case.kindex"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java:__subpackages__"],
)

java_verifier_test(
    name = "default_package_tests",
    size = "small",
    srcs = ["DefaultPackage.java"],
)
