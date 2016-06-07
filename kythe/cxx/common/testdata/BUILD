package(default_visibility = ["//kythe:default_visibility"])

filegroup(
    name = "start_http_service",
    srcs = ["start_http_service.sh"],
)

sh_test(
    name = "net_client_test_runner",
    size = "small",
    srcs = [
        "net_client_test_runner.sh",
    ],
    data = [
        "net_client_test_data.json",
        ":start_http_service",
        "//kythe/cxx/common:net_client_test",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/serving/tools:write_tables",
        "//kythe/go/storage/tools:write_entries",
        "//kythe/go/test/tools:http_server",
    ],
)
