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
    add_exports = [
        "jdk.compiler/com.sun.tools.javac.api",
        "jdk.compiler/com.sun.tools.javac.code",
        "jdk.compiler/com.sun.tools.javac.file",
        "jdk.compiler/com.sun.tools.javac.main",
        "jdk.compiler/com.sun.tools.javac.util",
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
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_protobuf//:protobuf_java_util",
        "@maven//:com_google_auto_auto_common",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:org_apache_tomcat_tomcat_annotations_api",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "system_exploder",
    srcs = ["SystemExploder.java"],
    deps = [
        "//third_party/guava",
    ],
)

java_binary(
    name = "explodesys",
    main_class = "com.google.devtools.kythe.extractors.java.SystemExploder",
    runtime_deps = [":system_exploder"],
)
