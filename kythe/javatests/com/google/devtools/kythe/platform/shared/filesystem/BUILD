load("@io_bazel_rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "CompilationUnitFileSystemTest",
    size = "small",
    srcs = ["CompilationUnitFileSystemTest.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared/filesystem",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@junit_junit//jar",
    ],
)
