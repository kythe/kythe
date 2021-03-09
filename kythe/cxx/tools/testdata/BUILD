load("//tools:build_rules/testing.bzl", "shell_tool_test")

package(default_visibility = ["//kythe:default_visibility"])

shell_tool_test(
    name = "test_claim_tool_kzip",
    data = [
        "claim_test.expected",
        "claim_test_1.kzip_UNIT.json",
        "claim_test_2.kzip_UNIT.json",
    ],
    scriptfile = "test_claim_tool_kzip.sh",
    tools = {
        "CLAIM_TOOL_BIN": "//kythe/cxx/tools:static_claim",
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
