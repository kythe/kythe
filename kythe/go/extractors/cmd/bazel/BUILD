load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "bazel_go_extractor",
    srcs = ["bazel_go_extractor/bazel_go_extractor.go"],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/extractors/golang",
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/vnameutil",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:go_go_proto",
        "//kythe/proto:storage_go_proto",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_shell//:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)

go_binary(
    name = "extract_kindex",
    srcs = ["extract_kindex/extract_kindex.go"],
    deps = [
        "//kythe/go/extractors/bazel",
        "//kythe/go/platform/kindex",
        "//kythe/go/util/vnameutil",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)

# An action listener that attaches the Go extractor action to Go compilations.
action_listener(
    name = "extract_kindex_go",
    extra_actions = [":extra_action"],
    mnemonics = ["GoCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = ("$(location :bazel_go_extractor)" +
           " $(EXTRA_ACTION_FILE)" +
           " $(output $(ACTION_ID).go.kindex)" +
           " $(location :govnames.json)"),
    data = [":govnames.json"],
    out_templates = ["$(ACTION_ID).go.kindex"],
    tools = [":bazel_go_extractor"],
)

load("//kythe/go/util/vnameutil:test/macros.bzl", "test_vname_rules")

test_vname_rules(
    name = "govnames_test",
    rules = "govnames.json",
    tests = "govnames_test.json",
)
