package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "file_vnames_test",
    srcs = ["FileVNamesTest.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/proto:storage_proto_java",
        "//third_party:junit4",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)

java_test(
    name = "extractor_utils_test",
    srcs = ["ExtractorUtilsTest.java"],
    data = ["testdata/sample.txt"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party:junit4",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)
