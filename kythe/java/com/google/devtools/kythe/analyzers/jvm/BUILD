package(
    default_visibility = ["//kythe:default_visibility"],
    licenses = ["notice"],
)

java_library(
    name = "jvm_graph",
    srcs = ["JvmGraph.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:common_java_proto",
        "//kythe/proto:storage_java_proto",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "analyzer",
    srcs = [
        "ClassFileIndexer.java",
        "KytheClassVisitor.java",
    ],
    deps = [
        ":jvm_graph",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:stream_fact_emitter",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:java_java_proto",
        "//kythe/proto:storage_java_proto",
        "@com_beust_jcommander//jar",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@org_ow2_asm_asm//jar",
    ],
)

java_binary(
    name = "class_file_indexer",
    main_class = "com.google.devtools.kythe.analyzers.jvm.ClassFileIndexer",
    runtime_deps = [":analyzer"],
)
