load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "bazel_go_extractor",
    srcs = ["bazel_go_extractor.go"],
    deps = [
        "@go_protobuf//:proto",
        "@go_shell//:shell",
        "@go_x_net//:context",
        "//kythe/go/extractors/bazel",
        "//kythe/go/extractors/golang",
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/storage/vnameutil",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/bazel:extra_actions_base_proto_go",
    ],
)

# An action listener that attaches the Go extractor action to Go compilations.
action_listener(
    name = "extract_kindex",
    extra_actions = [":extra_action"],
    mnemonics = ["GoCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = "$(location :bazel_go_extractor)" +
          " $(EXTRA_ACTION_FILE)" +
          " $(output $(ACTION_ID).go.kindex)" +
          " $(location //kythe/data:vnames_config)",
    data = ["//kythe/data:vnames_config"],
    out_templates = ["$(ACTION_ID).go.kindex"],
    tools = [":bazel_go_extractor"],
)
