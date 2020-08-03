load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/testing.bzl", "shell_tool_test")
load(":compile_commands.bzl", "compile_commands")

compile_commands(
    name = "compile_commands",
    filenames = [
        "nothing.cc",
        "basic.cc",
        "hopeless.cc",
    ],
)

shell_tool_test(
    name = "nothing",
    data = [
        "compile_commands.json.in",
        "nothing.cc",
        "nothing.cc.expected",
        "nothing.cc.json",
        "test_case.sh",
        "//kythe/cxx/common/testdata:start_http_service",
    ],
    scriptfile = "nothing_test.sh",
    tools = {
        "FYI": "//kythe/cxx/tools/fyi",
        "KYTHE_ENTRYSTREAM": "//kythe/go/platform/tools/entrystream",
        "KYTHE_HTTP_SERVER": "//kythe/go/test/tools:http_server",
        "KYTHE_WRITE_ENTRIES": "//kythe/go/storage/tools:write_entries",
        "KYTHE_WRITE_TABLES": "//kythe/go/serving/tools:write_tables",
    },
)

shell_tool_test(
    name = "basic",
    data = [
        "basic.cc",
        "basic.cc.expected",
        "basic.cc.json",
        "basic.h",
        "compile_commands.json.in",
        "test_case.sh",
        "//kythe/cxx/common/testdata:start_http_service",
    ],
    scriptfile = "basic_test.sh",
    tools = {
        "FYI": "//kythe/cxx/tools/fyi",
        "KYTHE_ENTRYSTREAM": "//kythe/go/platform/tools/entrystream",
        "KYTHE_HTTP_SERVER": "//kythe/go/test/tools:http_server",
        "KYTHE_WRITE_ENTRIES": "//kythe/go/storage/tools:write_entries",
        "KYTHE_WRITE_TABLES": "//kythe/go/serving/tools:write_tables",
    },
)

shell_tool_test(
    name = "hopeless",
    size = "small",
    data = [
        "compile_commands.json.in",
        "hopeless.cc",
        "hopeless.cc.json",
        "test_case.sh",
        "//kythe/cxx/common/testdata:start_http_service",
    ],
    scriptfile = "hopeless_test.sh",
    tools = {
        "FYI": "//kythe/cxx/tools/fyi",
        "KYTHE_ENTRYSTREAM": "//kythe/go/platform/tools/entrystream",
        "KYTHE_HTTP_SERVER": "//kythe/go/test/tools:http_server",
        "KYTHE_WRITE_ENTRIES": "//kythe/go/storage/tools:write_entries",
        "KYTHE_WRITE_TABLES": "//kythe/go/serving/tools:write_tables",
    },
)

bzl_library(
    name = "compile_commands_bzl",
    srcs = ["compile_commands.bzl"],
)
