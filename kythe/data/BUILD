package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/testing.bzl", "shell_tool_test")

exports_files(["schema_index.textproto"])

filegroup(
    name = "vnames_config",
    srcs = ["vnames.json"],
)

filegroup(
    name = "empty_vnames_config",
    srcs = ["empty_vnames.json"],
)

shell_tool_test(
    name = "filevnames_config",
    data = [":vnames_config"],
    scriptfile = "test_filevnames.sh",
    tools = {
        "DIR_INDEXER": "//kythe/go/storage/tools:directory_indexer",
    },
)
