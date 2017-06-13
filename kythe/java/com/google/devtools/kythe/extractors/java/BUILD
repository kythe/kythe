package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "java",
    srcs = [
        "InputUsageRecord.java",
        "JavaCompilationUnitExtractor.java",
        "ProcessAnnotation.java",
        "UsageAsInputReportingFileManager.java",
        "UsageAsInputReportingJavaFileObject.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/java/com/google/devtools/kythe/platform/java:options",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:buildinfo_proto_java",
        "//kythe/proto:java_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/javac",
        "//third_party/proto:any_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_guava_guava//jar",
    ],
)
