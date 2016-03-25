package(default_visibility = ["//kythe:default_visibility"])

action_listener(
    name = "extract_kindex",
    extra_actions = [":extra_action"],
    mnemonics = ["Javac"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = "$(location :java_extractor) $(EXTRA_ACTION_FILE) $(output $(ACTION_ID).java.kindex) $(location //kythe/data:vnames_config)",
    data = ["//kythe/data:vnames_config"],
    out_templates = ["$(ACTION_ID).java.kindex"],
    tools = [":java_extractor"],
)

java_binary(
    name = "java_extractor",
    srcs = [
        "JavaExtractor.java",
    ],
    main_class = "com.google.devtools.kythe.extractors.java.bazel.JavaExtractor",
    deps = [
        "//external:guava",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//third_party/bazel:extra_actions_base_proto_java",
    ],
)
