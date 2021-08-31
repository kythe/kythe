load("//tools:build_rules/testing.bzl", "shell_tool_test")

package(default_visibility = ["//kythe:default_visibility"])

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
        "KYTHE_HTTP_SERVER": "//kythe/go/test/tools/http_server",
        "KYTHE_WRITE_ENTRIES": "//kythe/go/storage/tools:write_entries",
        "KYTHE_WRITE_TABLES": "//kythe/go/serving/tools:write_tables",
    },
)
