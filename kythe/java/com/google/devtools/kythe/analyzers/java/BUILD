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
        "MarkedSources.java",
        "MiniAnchor.java",
        "NameVisitor.java",
        "SourceText.java",
        "TreeContext.java",
    ],
    deps = [
        ":plugin",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:config",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:schema",
        "//kythe/java/com/google/devtools/kythe/analyzers/jvm:jvm_graph",
        "//kythe/java/com/google/devtools/kythe/common:flogger",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/java/com/google/devtools/kythe/util:position_mappings",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:common_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
        "@com_beust_jcommander//jar",
        "@com_google_code_findbugs_jsr305//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "source_text",
    srcs = ["SourceText.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/util:position_mappings",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
    ],
)

java_library(
    name = "plugin",
    srcs = ["Plugin.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/platform/java/helpers",
        "//kythe/java/com/google/devtools/kythe/util:span",
        "//kythe/proto:storage_java_proto",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
    ],
)

java_binary(
    name = "indexer",
    srcs = ["JavaIndexer.java"],
    data = ["//third_party/javac:javac_jar"],
    jvm_flags = [
        "-Xbootclasspath/p:$${RUNPATH}$(location //third_party/javac:javac_jar)",
    ],
    main_class = "com.google.devtools.kythe.analyzers.java.JavaIndexer",
    deps = [
        ":analyzer",
        ":plugin",
        "//kythe/java/com/google/devtools/kythe/analyzers/base:stream_fact_emitter",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/platform/java",
        "//kythe/java/com/google/devtools/kythe/platform/shared:analysis_exception",
        "//kythe/java/com/google/devtools/kythe/platform/shared:file_data",
        "//kythe/java/com/google/devtools/kythe/platform/shared:metadata",
        "//kythe/java/com/google/devtools/kythe/platform/shared:statistics",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_beust_jcommander//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)
