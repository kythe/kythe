package(default_visibility = ["//kythe:default_visibility"])

filegroup(
    name = "analyzer_driver",
    srcs = ["//kythe/go/platform/tools/analyzer_driver"],
)

filegroup(
    name = "entrystream",
    srcs = ["//kythe/go/platform/tools/entrystream"],
)

filegroup(
    name = "ktags",
    srcs = ["//kythe/go/platform/tools/ktags"],
)

filegroup(
    name = "dedup_stream",
    srcs = ["//kythe/go/platform/tools/dedup_stream"],
)

filegroup(
    name = "viewindex",
    srcs = ["//kythe/go/platform/tools/viewindex"],
)

filegroup(
    name = "indexpack",
    srcs = ["//kythe/go/platform/tools/indexpack"],
)

sh_test(
    name = "analyzer_driver_test",
    size = "small",
    srcs = ["testdata/analyzer_driver_test.sh"],
    data = [
        ":analyzer_driver",
        "//kythe/go/platform/tools:entrystream",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer_server",
        "//kythe/testdata:test.kindex",
    ],
    tags = [
        "flaky",  # flaky due to port picking
        "local",
    ],
)

sh_test(
    name = "indexpack_test",
    size = "small",
    srcs = ["indexpack_test.sh"],
    data = [
        ":indexpack",
        ":viewindex",
        "//external:jq",
        "//kythe/testdata:test.kindex",
    ],
)
