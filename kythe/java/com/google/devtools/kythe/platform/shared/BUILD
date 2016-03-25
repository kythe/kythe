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
        "//external:proto/protobuf_java",
        "//kythe/proto:analysis_proto_java",
    ],
)

java_library(
    name = "remote_file_data",
    srcs = [
        "RemoteFileData.java",
    ],
    deps = [
        ":file_data",
        "//external:grpc-java",
        "//external:guava",
        "//external:proto/protobuf_java",
        "//kythe/proto:analysis_proto_java",
    ],
)
