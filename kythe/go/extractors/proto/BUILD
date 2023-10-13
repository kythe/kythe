load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "extract_proto_kzip",
    srcs = ["extract_proto_kzip.go"],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/util/log",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "@org_bitbucket_creachadair_stringset//:stringset",
    ],
)

action_listener(
    name = "extract_kzip",
    extra_actions = [":extra_action_kzip"],
    mnemonics = ["GenProtoDescriptorSet"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action_kzip",
    cmd = " ".join([
        "$(location :extract_proto_kzip)",
        "--extra_action=$(EXTRA_ACTION_FILE)",
        "--output=$(output $(ACTION_ID).proto.kzip)",
        "--rules=$(location //external:vnames_config)",
        "--language=protobuf",
    ]),
    data = ["//external:vnames_config"],
    out_templates = ["$(ACTION_ID).proto.kzip"],
    tools = [":extract_proto_kzip"],
)
