package(default_visibility = ["//visibility:public"])

java_binary(
    name = "bazel_jvm_extractor",
    srcs = ["BazelJvmExtractor.java"],
    main_class = "com.google.devtools.kythe.extractors.jvm.bazel.BazelJvmExtractor",
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/jvm:jvm_extractor",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//third_party/bazel:extra_actions_base_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
    ],
)
