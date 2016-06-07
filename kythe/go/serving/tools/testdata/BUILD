sh_binary(
    name = "debug_serving",
    srcs = ["debug_serving.sh"],
    data = [
        "//kythe/go/util/tools:scan_leveldb",
        "//third_party/jq",
    ],
)

sh_binary(
    name = "entries2tables",
    srcs = ["entries2tables.sh"],
    data = [
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/serving/tools:write_tables",
    ],
)

sh_test(
    name = "write_tables_test",
    size = "small",
    srcs = ["write_tables_test.sh"],
    data = [
        ":debug_serving",
        ":entries2tables",
        "//kythe/testdata:entries.gz",
        "//kythe/testdata:serving_table.decor.json.gz",
        "//kythe/testdata:serving_table.edgePages.json.gz",
        "//kythe/testdata:serving_table.edgeSets.json.gz",
        "//kythe/testdata:serving_table.xrefPages.json.gz",
        "//kythe/testdata:serving_table.xrefs.json.gz",
        "//third_party/jq",
    ],
)

sh_test(
    name = "kwazthis_test",
    size = "small",
    srcs = ["kwazthis_test.sh"],
    data = [
        "//external:jq",
        "//kythe/cxx/common/testdata:start_http_service",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/serving/tools:kwazthis",
        "//kythe/go/serving/tools:write_tables",
        "//kythe/go/storage/tools:write_entries",
        "//kythe/go/test/tools:http_server",
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata/pkg:generics_tests",
    ],
)
