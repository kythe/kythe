package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/testing.bzl", "shell_tool_test")

exports_files([
    "empty.kzip",
    "garbage_unit.kzip",
    "malformed.kzip",
    "missing-unit.kzip",
    "missing-pbunit.kzip",
    "stringset.kzip",
    "stringset_with_empty_file.kzip",
    "56bf5044e1b5c4c1cc7c4b131ac2fb979d288460e63352b10eef80ca35bd0a7b.pbtxt",
    "bb761979683e7c268e967eb5bcdedaa7fa5d1d472b0826b00b69acafbaad7ee6.pbtxt",
    "e9e170dcfca53c8126755bbc8b703994dedd3af32584291e01fba164ab5d3f32.pbtxt",
])

filegroup(
    name = "start_http_service",
    srcs = ["start_http_service.sh"],
)

shell_tool_test(
    name = "net_client_test_runner",
    data = [
        "net_client_test_data.json",
        ":start_http_service",
        "//kythe/cxx/common:net_client_test",
    ],
    scriptfile = "net_client_test_runner.sh",
    tools = {
        "KYTHE_ENTRYSTREAM": "//kythe/go/platform/tools/entrystream",
        "KYTHE_HTTP_SERVER": "//kythe/go/test/tools:http_server",
        "KYTHE_WRITE_ENTRIES": "//kythe/go/storage/tools:write_entries",
        "KYTHE_WRITE_TABLES": "//kythe/go/serving/tools:write_tables",
    },
)
