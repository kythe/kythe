package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/testing.bzl", "shell_tool_test")
load(":vnames.bzl", "construct_vnames_config")

exports_files([
    "schema_index.textproto",
    "vnames.bzl",
])

construct_vnames_config(
    name = "vnames_config",
    srcs = [":raw_vnames_config"],
    corpus = "kythe",
    visibility = ["//visibility:public"],
)

filegroup(
    name = "raw_vnames_config",
    srcs = [
        "vnames.cxx.json",
        "vnames.go.json",
        "vnames.java.json",
        "vnames.json",
    ],
)

filegroup(
    name = "empty_vnames_config",
    srcs = ["empty_vnames.json"],
)

shell_tool_test(
    name = "filevnames_config",
    args = ["$(location :vnames_config)"],
    data = [":vnames_config"],
    scriptfile = "test_filevnames.sh",
    tools = {
        "DIR_INDEXER": "//kythe/go/storage/tools:directory_indexer",
    },
)

load("//kythe/go/util/vnameutil:test/macros.bzl", "test_vname_rules")

test_vname_rules(
    name = "go_vnames_test",
    rules = "vnames.go.json",
    tests = "vnames_test.go.json",
)
