package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "java",
    srcs = [
        "JavaCompilationDetails.java",
        "JavacAnalysisDriver.java",
        "JavacAnalyzer.java",
        "JavacOptionsUtils.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager",
        "//kythe/java/com/google/devtools/kythe/platform/shared",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/guava",
        "//third_party/jsr305_annotations:jsr305",
        "//third_party/openjdk:tools",
        "//third_party/proto:protobuf_java",
    ],
)
