package(default_visibility = ["//visibility:public"])

java_library(
    name = "analyzer",
    srcs = [
        "FilePositions.java",
        "JavaEntrySets.java",
        "KytheJavacAnalyzer.java",
        "KytheTreeScanner.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/platform/shared",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/guava",
        "//third_party/openjdk/lib:tools_jar",
        "//third_party/proto:protobuf_java",
    ],
)

java_binary(
    name = "indexer",
    srcs = [
        "JavaIndexer.java",
    ],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexer",
    deps = [
        ":analyzer",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)

java_binary(
    name = "indexer_server",
    srcs = [
        "JavaIndexerServer.java",
    ],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexerServer",
    deps = [
        ":analyzer",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:compilation_analyzer",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/proto:analysis_proto",
        "//third_party/grpc-java",
        "//third_party/guava",
        "//third_party/netty",
    ],
)
