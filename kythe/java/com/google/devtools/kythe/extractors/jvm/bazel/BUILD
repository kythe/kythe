package(default_visibility = ["//kythe:default_visibility"])

action_listener(
    name = "extract_kindex",
    extra_actions = [":extra_action"],
    mnemonics = ["JavaIjar"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = "$(location :bazel_jvm_extractor) $(EXTRA_ACTION_FILE) $(output $(ACTION_ID).jvm.kindex) $(location //external:vnames_config)",
    data = ["//external:vnames_config"],
    out_templates = ["$(ACTION_ID).jvm.kindex"],
    tools = [":bazel_jvm_extractor"],
)

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
