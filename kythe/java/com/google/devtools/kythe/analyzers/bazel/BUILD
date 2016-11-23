package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "LabelUtil",
    srcs = ["LabelUtil.java"],
)

java_library(
    name = "Tickets",
    srcs = ["Tickets.java"],
    deps = [
        ":LabelUtil",
        "//kythe/java/com/google/devtools/kythe/util",
        "@com_google_auto_value//jar",
        "@com_google_guava_guava//jar",
        "@io_bazel//src/main/java/com/google/devtools/build/lib:skylark-lang",
    ],
)

java_binary(
    name = "indexer",
    srcs = ["BazelIndexer.java"],
    main_class = "com.google.devtools.kythe.analyzers.bazel.BazelIndexer",
    deps = [
        ":LabelUtil",
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
