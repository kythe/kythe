package(default_visibility = ["//kythe:default_visibility"])

sh_test(
    name = "nothing",
    size = "small",
    srcs = [
        "nothing_test.sh",
    ],
    data = [
        "compile_commands.json.in",
        "nothing.cc",
        "nothing.cc.expected",
        "nothing.cc.json",
        "test_case.sh",
        "//kythe/cxx/common/testdata:start_http_service",
        "//kythe/cxx/tools/fyi",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/serving/tools:write_tables",
        "//kythe/go/storage/tools:write_entries",
        "//kythe/go/test/tools:http_server",
    ],
)

sh_test(
    name = "basic",
    size = "small",
    srcs = [
        "basic_test.sh",
    ],
    data = [
        "basic.cc",
        "basic.cc.expected",
        "basic.cc.json",
        "basic.h",
        "compile_commands.json.in",
        "test_case.sh",
        "//kythe/cxx/common/testdata:start_http_service",
        "//kythe/cxx/tools/fyi",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/serving/tools:write_tables",
        "//kythe/go/storage/tools:write_entries",
        "//kythe/go/test/tools:http_server",
    ],
)

sh_test(
    name = "hopeless",
    size = "small",
    srcs = [
        "hopeless_test.sh",
    ],
    data = [
        "compile_commands.json.in",
        "hopeless.cc",
        "hopeless.cc.json",
        "test_case.sh",
        "//kythe/cxx/common/testdata:start_http_service",
        "//kythe/cxx/tools/fyi",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/serving/tools:write_tables",
        "//kythe/go/storage/tools:write_entries",
        "//kythe/go/test/tools:http_server",
    ],
)
