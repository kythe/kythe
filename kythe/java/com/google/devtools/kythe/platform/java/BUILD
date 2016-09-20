package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "java",
    srcs = [
        "JavaCompilationDetails.java",
        "JavacAnalysisDriver.java",
        "JavacAnalyzer.java",
    ],
    deps = [
        ":options",
        "//external:guava",
        "//external:jsr305",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
    ],
)

java_library(
    name = "options",
    srcs = ["JavacOptionsUtils.java"],
    deps = [
        "//external:guava",
        "//external:jsr305",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
    ],
)
