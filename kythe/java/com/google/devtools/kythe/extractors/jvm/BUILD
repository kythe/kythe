load("@rules_java//java:defs.bzl", "java_library", "java_binary")

package(default_visibility = ["//kythe:default_visibility"])

java_binary(
    name = "jar_extractor",
    srcs = ["JarExtractor.java"],
    main_class = "com.google.devtools.kythe.extractors.jvm.JarExtractor",
    deps = [
        ":jvm_extractor",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//third_party/guava",
        "@maven//:com_beust_jcommander",
    ],
)

java_library(
    name = "jvm_extractor",
    srcs = ["JvmExtractor.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/jvm:jvm_graph",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:buildinfo_java_proto",
        "//kythe/proto:java_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_protobuf//:protobuf_java_util",
        "@maven//:com_beust_jcommander",
    ],
)
