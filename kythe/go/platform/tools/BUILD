package(default_visibility = ["//kythe:default_visibility"])

sh_test(
    name = "indexpack_test",
    size = "small",
    srcs = ["indexpack_test.sh"],
    data = [
        "//kythe/go/platform/tools/indexpack",
        "//kythe/go/platform/tools/viewindex",
        "//kythe/testdata:test.kindex",
        "//third_party/jq",
    ],
)
