package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/testing.bzl", "shell_tool_test")

shell_tool_test(
    name = "indexpack_test",
    scriptfile = "indexpack_test.sh",
    tools = {
        "indexpack": "//kythe/go/platform/tools/indexpack",
        "viewindex": "//kythe/go/platform/tools/viewindex",
        "input": "//kythe/testdata:test.kindex",
        "jq": "//third_party/jq",
    },
)
