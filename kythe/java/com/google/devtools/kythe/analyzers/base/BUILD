package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "stream_fact_emitter",
    srcs = ["StreamFactEmitter.java"],
    deps = [
        ":entryset",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
    ],
)

java_library(
    name = "config",
    srcs = ["IndexerConfig.java"],
    deps = ["@com_beust_jcommander//jar"],
)

java_library(
    name = "schema",
    srcs = [
        "EdgeKind.java",
        "NodeKind.java",
    ],
    deps = [
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_guava_guava//jar",
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
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:common_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_guava_guava//jar",
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
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
    ],
)

java_library(
    name = "grpc_compilation_analyzer",
    srcs = [
        "GRPCCompilationAnalyzer.java",
    ],
    deps = [
        ":compilation_analyzer",
        ":entryset",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:remote_file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:analysis_service_proto_java",
        "//third_party/grpc-java",
    ],
)

# //kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata:corner_case.kindex is derived
# based on the sources in //kythe/java/com/google/devtools/kythe/analyzers/java:analyzer which
# depend on this class.
filegroup(
    name = "corner_case_srcs",
    srcs = ["KytheEntrySets.java"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java:__subpackages__"],
)
