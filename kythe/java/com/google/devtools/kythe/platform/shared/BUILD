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
        "//external:guava",
        "//kythe/proto:analysis_proto_java",
        "//third_party/proto:protobuf_java",
    ],
)

java_library(
    name = "remote_file_data",
    srcs = [
        "RemoteFileData.java",
    ],
    deps = [
        ":file_data",
        "//external:guava",
        "//kythe/proto:analysis_proto_java",
        "//third_party/grpc-java",
        "//third_party/proto:protobuf_java",
    ],
)
