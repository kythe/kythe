package(default_visibility = ["//kythe:default_visibility"])

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

sh_test(
    name = "test_claim_tool_kindex",
    size = "small",
    srcs = [
        "test_claim_tool_kindex.sh",
    ],
    data = [
        "claim_test.expected",
        "claim_test_1.kindex_UNIT",
        "claim_test_2.kindex_UNIT",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/cxx/tools:static_claim",
    ],
)

sh_test(
    name = "test_claim_tool_index_pack",
    size = "small",
    srcs = [
        "test_claim_tool_index_pack.sh",
    ],
    data = [
        "claim_test.expected",
        "claim_test_1.kindex_UNIT",
        "claim_test_2.kindex_UNIT",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/cxx/tools:static_claim",
        "//kythe/go/platform/tools:indexpack",
    ],
)
