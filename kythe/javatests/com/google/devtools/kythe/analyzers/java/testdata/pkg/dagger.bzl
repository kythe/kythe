load("//tools/build_rules/verifier_test:verifier_test.bzl", "java_verifier_test")

def dagger_verifier_tests(srcs):
  for src in srcs:
    java_verifier_test(
        name = src.replace(".java", "Test"),
        size = "small",
        srcs = [src] + native.glob(["daggerstubs/*.java"]),
        load_plugin = "//kythe/java/dagger/internal/codegen:dagger-kythe-plugin",
        verifier_opts = [
            "--convert_marked_source",
            "--ignore_dups",
        ],
    )
