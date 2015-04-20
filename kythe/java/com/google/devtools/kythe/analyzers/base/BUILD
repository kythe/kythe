package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "schema",
    srcs = [
        "EdgeKind.java",
        "NodeKind.java",
    ],
)

java_library(
    name = "entryset",
    srcs = [
        "CorpusPath.java",
        "EntrySet.java",
        "FactEmitter.java",
        "KytheEntrySets.java",
    ],
    deps = [
        ":schema",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)

java_library(
    name = "compilation_analyzer",
    srcs = [
        "AbstractCompilationAnalyzer.java",
    ],
    deps = [
        ":entryset",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/shared",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:remote_file_data",
        "//kythe/proto:analysis_proto",
        "//kythe/proto:storage_proto",
        "//third_party/grpc-java",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)
