sh_test(
    name = "indexer_test",
    srcs = ["indexer_test.sh"],
    data = [
        "//kythe/go/platform/tools:entrystream",
        "//kythe/go/platform/tools:indexpack",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
        "//kythe/testdata:test.kindex",
    ],
)

java_test(
    name = "file_positions_test",
    srcs = ["FilePositionsTest.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/java:analyzer",
        "//third_party:junit4",
        "//third_party/truth",
    ],
)
