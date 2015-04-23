package(default_visibility = ["//kythe:default_visibility"])

sh_test(
    name = "kindex",
    srcs = [
        "test_kindex.sh",
    ],
    data = [
        "kindex_repo_test.header",
        "kindex_repo_test.main",
        "kindex_repo_test.unit",
        "kindex_repo_test.verify",
        "kindex_test.header",
        "kindex_test.main",
        "kindex_test.unit",
        "kindex_test.verify",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/cxx/verifier",
    ],
)

sh_test(
    name = "index_pack",
    srcs = [
        "test_index_pack.sh",
    ],
    data = [
        "kindex_repo_test.header",
        "kindex_repo_test.main",
        "kindex_repo_test.unit",
        "kindex_repo_test.verify",
        "kindex_test.header",
        "kindex_test.main",
        "kindex_test.unit",
        "kindex_test.verify",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/tools:kindex_tool",
        "//kythe/cxx/verifier",
        "//kythe/go/platform/tools:indexpack",
    ],
)

sh_test(
    name = "bundle",
    srcs = [
        "test_bundle.sh",
    ],
    data = [
        "bundle_self_test.cc",
        "bundle_self_test_mix.cc",
        "bundle_self_test_multi_transcript.cc",
        "bundle_self_test_unclaimed.cc",
        "bundle_self_test_vnames_json.cc",
        "claim_macro_features.cc",
        "test_vnames.json",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/verifier",
    ],
)

sh_test(
    name = "indexer_basic",
    srcs = [
        "test_basic.sh",
    ],
    data = glob(["basic/*"]) + [
        "//kythe/cxx/indexer/cxx:indexer",
        "//kythe/cxx/verifier:verifier",
    ],
)
