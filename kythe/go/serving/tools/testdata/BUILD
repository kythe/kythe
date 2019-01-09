load("//tools:build_rules/testing.bzl", "shell_tool_test")

sh_binary(
    name = "debug_serving",
    srcs = ["debug_serving.sh"],
    data = [
        "//kythe/go/util/tools/scan_leveldb",
        "@com_github_stedolan_jq//:jq",
    ],
)

sh_binary(
    name = "entries2tables",
    srcs = ["entries2tables.sh"],
    data = [
        "//kythe/go/platform/tools/entrystream",
        "//kythe/go/serving/tools:write_tables",
    ],
)

shell_tool_test(
    name = "write_tables_test",
    data = [
        "//kythe/testdata:entries.gz",
        "//kythe/testdata:serving_table.decor.json.gz",
        "//kythe/testdata:serving_table.edgePages.json.gz",
        "//kythe/testdata:serving_table.edgeSets.json.gz",
        "//kythe/testdata:serving_table.xrefPages.json.gz",
        "//kythe/testdata:serving_table.xrefs.json.gz",
    ],
    scriptfile = "write_tables_test.sh",
    tools = {
        "entrystream": "//kythe/go/platform/tools/entrystream",
        "jq": "@com_github_stedolan_jq//:jq",
        "scan_leveldb": "//kythe/go/util/tools/scan_leveldb",
        "write_tables": "//kythe/go/serving/tools:write_tables",
    },
)

shell_tool_test(
    name = "kwazthis_test",
    data = [
        "//kythe/cxx/common/testdata:start_http_service",
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg:generics_tests_entries.entries.gz",
    ],
    scriptfile = "kwazthis_test.sh",
    tools = {
        "JQ": "@com_github_stedolan_jq//:jq",
        "KWAZTHIS": "//kythe/go/serving/tools:kwazthis",
        "KYTHE_ENTRYSTREAM": "//kythe/go/platform/tools/entrystream",
        "KYTHE_HTTP_SERVER": "//kythe/go/test/tools:http_server",
        "KYTHE_WRITE_ENTRIES": "//kythe/go/storage/tools:write_entries",
        "KYTHE_WRITE_TABLES": "//kythe/go/serving/tools:write_tables",
    },
)
