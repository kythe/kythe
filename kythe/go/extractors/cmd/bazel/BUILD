load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

filegroup(
    name = "bazel_go_extractor",
    srcs = ["//kythe/go/extractors/cmd/bazel/bazel_go_extractor"],
)

filegroup(
    name = "extract_kindex",
    srcs = ["//kythe/go/extractors/cmd/bazel/extract_kindex"],
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
           " -corpus kythe.io" +
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
