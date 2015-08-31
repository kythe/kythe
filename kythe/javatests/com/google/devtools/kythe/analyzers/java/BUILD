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
    name = "source_text_test",
    srcs = ["SourceTextTest.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/java:analyzer",
        "//third_party:junit4",
        "//third_party/truth",
    ],
)
