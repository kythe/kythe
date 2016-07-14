package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "config",
    srcs = ["IndexerConfig.java"],
    deps = ["//external:jcommander"],
)

java_library(
    name = "analyzer",
    srcs = [
        "JavaEntrySets.java",
        "KytheDocTreeScanner.java",
        "KytheJavacAnalyzer.java",
        "KytheTreeScanner.java",
        "MiniAnchor.java",
        "NameVisitor.java",
        "SourceText.java",
    ],
    deps = [
        "@bazel_tools//third_party/java/jdk/langtools:javac",
        ":config",
        "//external:guava",
        "//external:proto/protobuf_java",
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
    ],
)

java_binary(
    name = "indexer",
    srcs = ["JavaIndexer.java"],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexer",
    deps = [
        ":analyzer",
        ":config",
        "//external:guava",
        "//external:jcommander",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
    ],
)

java_binary(
    name = "indexer_server",
    srcs = ["JavaIndexerServer.java"],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexerServer",
    deps = [
        ":analyzer",
        ":config",
        "//external:grpc-java",
        "//external:guava",
        "//external:jcommander",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:grpc_compilation_analyzer",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
    ],
)
