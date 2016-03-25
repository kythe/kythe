package(default_visibility = ["//kythe:default_visibility"])

sh_test(
    name = "plan_ts_repo",
    size = "large",
    srcs = ["//kythe/cxx/tools/meta_index/testdata:plan_ts_repo_test.sh"],
    args = [],
    data = [
        "//external:jq",
        "//kythe/cxx/tools/meta_index",
        "//kythe/cxx/tools/meta_index/testdata:expected_entries.verifier",
        "//kythe/cxx/tools/meta_index/testdata:expected_script.sh",
        "//kythe/cxx/tools/meta_index/testdata:expected_vnames.json",
        "//kythe/cxx/verifier",
        "//kythe/ts/testdata:repo_files",
    ],
)
