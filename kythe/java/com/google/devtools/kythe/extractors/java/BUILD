load("@rules_java//java:defs.bzl", "java_library")

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
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/java/com/google/devtools/kythe/platform/java:options",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager:forwarding_standard_java_file_manager",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/util:delete_recursively",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:buildinfo_java_proto",
        "//kythe/proto:java_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_protobuf//:protobuf_java_util",
        "@maven//:com_google_auto_auto_common",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:javax_annotation_jsr250_api",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "system_exploder",
    srcs = ["SystemExploder.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager:forwarding_standard_java_file_manager",
        "//third_party/guava",
        "//third_party/javac",
    ],
)

java_binary(
    name = "explodesys",
    main_class = "com.google.devtools.kythe.extractors.java.SystemExploder",
    runtime_deps = [":system_exploder"],
)
