package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "config",
    srcs = ["IndexerConfig.java"],
    deps = ["//third_party/jcommander"],
)

java_library(
    name = "analyzer",
    srcs = [
        "JavaEntrySets.java",
        "KytheDocTreeScanner.java",
        "KytheJavacAnalyzer.java",
        "KytheTreeScanner.java",
        "SourceText.java",
    ],
    deps = [
        ":config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/guava",
        "//third_party/openjdk:tools",
        "//third_party/proto:protobuf_java",
    ],
)

java_binary(
    name = "indexer",
    srcs = ["JavaIndexer.java"],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexer",
    deps = [
        ":analyzer",
        ":config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/guava",
        "//third_party/jcommander",
        "//third_party/proto:protobuf_java",
    ],
)

java_binary(
    name = "indexer_server",
    srcs = ["JavaIndexerServer.java"],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexerServer",
    deps = [
        ":analyzer",
        ":config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:grpc_compilation_analyzer",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//third_party/grpc-java",
        "//third_party/guava",
        "//third_party/jcommander",
    ],
)
