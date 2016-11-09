package(default_visibility = ["//kythe:default_visibility"])

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
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
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
        "//third_party/proto:protobuf_java",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
        "@com_google_guava_guava//jar",
    ],
)

java_binary(
    name = "indexer",
    srcs = ["JavaIndexer.java"],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexer",
    deps = [
        ":analyzer",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_beust_jcommander//jar",
        "@com_google_guava_guava//jar",
    ],
)

java_binary(
    name = "indexer_server",
    srcs = ["JavaIndexerServer.java"],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexerServer",
    deps = [
        ":analyzer",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:grpc_compilation_analyzer",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//third_party/grpc-java",
        "@com_beust_jcommander//jar",
        "@com_google_guava_guava//jar",
    ],
)
