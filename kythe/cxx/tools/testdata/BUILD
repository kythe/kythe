package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/testing.bzl", "shell_tool_test")

sh_test(
    name = "test_kindex_tool",
    size = "small",
    srcs = [
        "test_kindex_tool.sh",
    ],
    data = [
        "java.kindex_UNIT",
        "java.kindex_cf28b786fa21d0c45156e8011ac809afc454703fa03d767a5aeeed382f902795",
        "//kythe/cxx/tools:kindex_tool",
    ],
)

shell_tool_test(
    name = "test_claim_tool_kindex",
    data = [
        "claim_test.expected",
        "claim_test_1.kindex_UNIT",
        "claim_test_2.kindex_UNIT",
    ],
    scriptfile = "test_claim_tool_kindex.sh",
    tools = {
        "CLAIM_TOOL_BIN": "//kythe/cxx/tools:static_claim",
        "KINDEX_TOOL_BIN": "//kythe/cxx/tools:kindex_tool",
    },
)

shell_tool_test(
    name = "test_claim_tool_index_pack",
    data = [
        "claim_test.expected",
        "claim_test_1.kindex_UNIT",
        "claim_test_2.kindex_UNIT",
    ],
    scriptfile = "test_claim_tool_index_pack.sh",
    tools = {
        "CLAIM_TOOL_BIN": "//kythe/cxx/tools:static_claim",
        "INDEX_PACK_BIN": "//kythe/go/platform/tools/indexpack",
        "KINDEX_TOOL_BIN": "//kythe/cxx/tools:kindex_tool",
    },
)

# TODO(#2375): (closed?) requires declarations generated in pipeline
# sh_test(
#     name = "def_decl_test",
#     srcs = ["def_decl_test.sh"],
#     data = [
#         "def_decl_test.cc",
#         "//kythe/cxx/indexer/cxx:indexer",
#         "//kythe/go/serving/tools:kythe",
#         "//kythe/go/serving/tools:write_tables",
#         "//kythe/go/storage/tools/write_entries",
#     ],
#     tags = ["manual"],  # Currently failing.
# )
