load("@rules_java//java:defs.bzl", "java_library")

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
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "options",
    srcs = ["JavacOptionsUtils.java"],
    deps = [
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:java_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@org_checkerframework_checker_qual//jar",
    ],
)
