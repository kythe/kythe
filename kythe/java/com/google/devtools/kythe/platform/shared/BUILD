package(default_visibility = ["//visibility:public"])

java_library(
    name = "file_data",
    srcs = [
        "FileDataCache.java",
        "FileDataDirectory.java",
        "FileDataProvider.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/proto:analysis_proto",
        "//third_party:protobuf",
        "//third_party/guava",
    ],
)

java_library(
    name = "shared",
    srcs = [
        "AnalysisException.java",
        "NullStatisticsCollector.java",
        "StatisticsCollector.java",
    ],
)
