package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "analyzer",
    srcs = [
        "JavaEntrySets.java",
        "JavaIndexerConfig.java",
        "JavaNode.java",
        "KytheDocTreeScanner.java",
        "KytheJavacAnalyzer.java",
        "KytheTreeScanner.java",
        "MiniAnchor.java",
        "NameVisitor.java",
        "SourceText.java",
        "TreeContext.java",
    ],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/java/com/google/devtools/kythe/util:position_mappings",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:common_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/javac",
        "//third_party/proto:protobuf_java",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
        "@com_beust_jcommander//jar",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_guava_guava//jar",
    ],
)

java_binary(
    name = "indexer",
    srcs = ["JavaIndexer.java"],
    data = [
        "//third_party/javac:javac_jar",
    ],
    jvm_flags = [
        "-Xbootclasspath/p:$${RUNPATH}$(location //third_party/javac:javac_jar)",
    ],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexer",
    deps = [
        ":analyzer",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:stream_fact_emitter",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_beust_jcommander//jar",
        "@com_google_guava_guava//jar",
    ],
)

# When run from within Bazel, we need to execute the Java indexer
# from a different directory and remap the provided paths as it refuses
# to run from the source directory and believes the execroot is one.
# See https://kythe.io/phabricator/T70
sh_binary(
    name = "wrapped_indexer",
    srcs = ["wrapped_indexer.sh"],
    data = [
        ":indexer",
        "//tools/cdexec",
        "//tools/modules:abspath",
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
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_proto_java",
        "//third_party/grpc-java",
        "@com_beust_jcommander//jar",
        "@com_google_guava_guava//jar",
    ],
)
