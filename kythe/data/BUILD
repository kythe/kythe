package(default_visibility = ["//kythe:default_visibility"])

filegroup(
    name = "vnames_config",
    srcs = ["vnames.json"],
)

filegroup(
    name = "empty_vnames_config",
    srcs = ["empty_vnames.json"],
)

sh_test(
    name = "filevnames_config",
    size = "small",
    srcs = ["test_filevnames.sh"],
    data = [
        ":vnames_config",
        "//kythe/go/storage/tools:directory_indexer",
    ],
)
