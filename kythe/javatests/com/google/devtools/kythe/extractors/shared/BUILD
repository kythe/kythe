package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "file_vnames_test",
    size = "small",
    srcs = ["FileVNamesTest.java"],
    test_class = "com.google.devtools.kythe.extractors.shared.FileVNamesTest",
    deps = [
        "//external:guava",
        "//external:junit4",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/proto:storage_proto_java",
    ],
)

java_test(
    name = "extractor_utils_test",
    size = "small",
    srcs = ["ExtractorUtilsTest.java"],
    data = ["testdata/sample.txt"],
    test_class = "com.google.devtools.kythe.extractors.shared.ExtractorUtilsTest",
    deps = [
        "//external:guava",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party:junit4",
    ],
)
