load("//tools/build_rules/verifier_test:verifier_test.bzl", "jvm_verifier_test")

exports_files(
    ["jvm_nodes.kythe_verifier.txt"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg:__pkg__"],
)

jvm_verifier_test(
    name = "jvm_nodes_tests",
    srcs = [
        "jvm_edges.kythe_verifier.txt",
        "jvm_nodes.kythe_verifier.txt",
    ],
    deps = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg:jvm_tests"],
)
