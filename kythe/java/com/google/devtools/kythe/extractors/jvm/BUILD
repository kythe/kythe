package(default_visibility = ["//kythe:default_visibility"])

java_binary(
    name = "jar_extractor",
    srcs = ["JarExtractor.java"],
    main_class = "com.google.devtools.kythe.extractors.jvm.JarExtractor",
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/jvm:jvm_graph",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)
