load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//visibility:public"])

java_library(
    name = "analysis_exception",
    srcs = ["AnalysisException.java"],
)

java_library(
    name = "statistics",
    srcs = [
        "MemoryStatisticsCollector.java",
        "NullStatisticsCollector.java",
        "StatisticsCollector.java",
    ],
)

java_library(
    name = "file_data",
    srcs = [
        "FileDataCache.java",
        "FileDataDirectory.java",
        "FileDataProvider.java",
    ],
    deps = [
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "metadata",
    srcs = [
        "KytheInlineMetadataLoader.java",
        "KytheMetadataLoader.java",
        "Metadata.java",
        "MetadataLoader.java",
        "MetadataLoaders.java",
        "ProtobufMetadataLoader.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:metadata_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:com_google_code_gson_gson",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "units",
    srcs = [
        "CompilationUnits.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_code_findbugs_jsr305",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "test_util",
    testonly = 1,
    srcs = ["TestDataUtil.java"],
)
