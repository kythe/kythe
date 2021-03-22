load("@io_bazel_rules_scala//scala:scala.bzl", "scala_test")

def gen_verifier_tests():
    for path in native.glob(["testdata/verified/*.scala"]):
        scala_test(
            name = path[path.rfind("/") + 1:].replace(".", "_") + "_verify",
            size = "small",
            jvm_flags = [
                "-Dscala.library.location=$(location @io_bazel_rules_scala_scala_library)",
                "-Ddedup.location=$(location //kythe/go/platform/tools/dedup_stream)",
                "-Dverifier.location=$(location //kythe/cxx/verifier)",
            ],
            data = [
                ":plugin",
                "//kythe/cxx/verifier",
                "//kythe/go/platform/tools/dedup_stream",
                "@io_bazel_rules_scala_scala_library",
            ] + ["//kythe/scala/com/google/devtools/kythe/analyzers/scala:" + path],
            deps = ["//kythe/scala/com/google/devtools/kythe/analyzers/scala:verifier_test_base"],
        )
