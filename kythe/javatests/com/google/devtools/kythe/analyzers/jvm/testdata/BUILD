load("//tools/build_rules/verifier_test:jvm_verifier_test.bzl", "jvm_verifier_test")

exports_files(
    ["jvm_nodes.kythe_verifier.txt"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg:__pkg__"],
)

jvm_verifier_test(
    name = "jvm_nodes_tests",
    size = "large",
    srcs = [
        "jvm_edges.kythe_verifier.txt",
        "jvm_nodes.kythe_verifier.txt",
    ],
    verifier_opts = [
        "--ignore_dups",
        "--convert_marked_source",
        # Ignore singletons that are used in the Java indexer side of the JVM tests.
        "--nocheck_for_singletons",
        "--nofile_vnames",
    ],
    deps = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg:jvm_tests"],
)
