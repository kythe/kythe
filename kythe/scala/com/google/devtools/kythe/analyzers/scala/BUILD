package(default_visibility = ["//kythe:default_visibility"])

load(
    "@io_bazel_rules_scala//scala:scala.bzl",
    "scala_binary",
    "scala_library",
    "scala_macro_library",
    "scala_test",
)
load(":verifier.bzl", "gen_verifier_tests")

TEST_SRCS = ["VerifierTest.scala"]

scala_library(
    name = "analyzer",
    srcs = glob(
        ["*.scala"],
        exclude = TEST_SRCS,
    ),
    deps = [
        "//external:io_bazel_rules_scala/dependency/scala/scala_compiler",
        "//kythe/java/com/google/devtools/kythe/util:normalizer",
        "//kythe/java/com/google/devtools/kythe/util/schema",
        "//kythe/proto:common_java_proto",
        "//kythe/proto:schema_java_proto",
        "//kythe/proto:serving_java_proto",
        "//kythe/proto:storage_java_proto",
        "@com_google_protobuf//:protobuf_java",
    ],
)

scala_library(
    name = "verifier_test_base",
    srcs = TEST_SRCS,
    deps = [
        "@io_bazel_rules_scala//testing/toolchain:scalatest_classpath",
        "@io_bazel_rules_scala//third_party/utils/src/test:test_util",
        "@io_bazel_rules_scala_scala_compiler",
        "@io_bazel_rules_scala_scala_library",
    ],
)

gen_verifier_tests()

# Put the jar exactly where we need it (at the top level)
# TODO(rbraunstein): update the scala.bzl file do take an optional plugin file
# or create scala_library_compiler_plugin rule that delegates to scala_library
genrule(
    name = "plugin",
    srcs = [
        ":analyzer_deploy.jar",
        "scalac-plugin.xml",
    ],
    outs = ["kythe-plugin.jar"],
    cmd = "$(location add_to_jar) $@ $(SRCS)",
    tools = [
        ":add_to_jar",
        # We have to make jar available here, sh_binary won't take it as deps
        # and data puts it under runfiles
        "@bazel_tools//tools/jdk",
    ],
)

sh_binary(
    name = "add_to_jar",
    srcs = ["add_to_jar.sh"],
)
