package(default_visibility = ["//visibility:public"])

sh_test(
    name = "compilation_database",
    size = "small",
    srcs = [
        "test_extract_compilation_database.sh",
    ],
    data = [
        "extract_compilation_database.sh",
        "testdata/compilation_database.json",
        "testdata/expected.file",
        "testdata/expected.unit",
        "testdata/test_file.cc",
        "//external:jq",
        "//kythe/cxx/extractor:cxx_extractor",
        "//kythe/cxx/tools:kindex_tool",
    ],
)
