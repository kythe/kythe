# Build file for tools integrating Bazel with clang tooling.

# Extracts a single compile command from an extra action.
cc_binary(
    name = "extract_compile_command",
    srcs = ["extract_compile_command.cc"],
    deps = [
        "//third_party/bazel:extra_actions_base_proto_cc",
        "//third_party/proto:protobuf",
        "//third_party/rapidjson",
        "@com_github_gflags_gflags//:gflags",
        "@com_github_google_glog//:glog",
    ],
)

action_listener(
    name = "extract_json",
    extra_actions = [":extra_action"],
    mnemonics = ["CppCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = "$(location :extract_compile_command) \
        $(EXTRA_ACTION_FILE) \
        $(output $(ACTION_ID).compile_command.json)",
    out_templates = ["$(ACTION_ID).compile_command.json"],
    tools = [":extract_compile_command"],
)
