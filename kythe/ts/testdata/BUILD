package(default_visibility = ["//kythe:default_visibility"])

load("ts_indexer_test", "ts_indexer_test")

filegroup(
    name = "repo_files",
    srcs = glob(["repo/**/*"]),
)

sh_binary(
    name = "run_case",
    srcs = ["run_case.sh"],
    data = [
        "//kythe/ts:ts_indexer",
        "//kythe/go/platform/tools:entrystream",
        "//tools:node",
        "//kythe/cxx/verifier",
    ],
)

test_suite(
    name = "basic_tests",
    tags = ["basic"],
)

ts_indexer_test(
    name = "var_ref",
    srcs = ["basic/var_ref.js"],
    tags = ["basic"],
)
