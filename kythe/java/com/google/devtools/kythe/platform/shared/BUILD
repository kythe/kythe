package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "analysis_exception",
    srcs = [
        "AnalysisException.java",
    ],
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
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/proto:analysis_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
    ],
)

java_library(
    name = "remote_file_data",
    srcs = [
        "RemoteFileData.java",
    ],
    deps = [
        ":file_data",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:analysis_service_proto_java",
        "//third_party/grpc-java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
    ],
)

java_library(
    name = "metadata",
    srcs = [
        "KytheMetadataLoader.java",
        "Metadata.java",
        "MetadataLoader.java",
        "MetadataLoaders.java",
        "ProtobufMetadataLoader.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/common:autovalue",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_code_gson_gson//jar",
        "@com_google_guava_guava//jar",
    ],
)
