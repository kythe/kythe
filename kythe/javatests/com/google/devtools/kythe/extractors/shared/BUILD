load("@rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "file_vnames_test",
    size = "small",
    srcs = ["FileVNamesTest.java"],
    test_class = "com.google.devtools.kythe.extractors.shared.FileVNamesTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/shared:file_vnames",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:junit_junit",
    ],
)

java_test(
    name = "extractor_utils_test",
    size = "small",
    srcs = ["ExtractorUtilsTest.java"],
    data = ["testdata/sample.txt"],
    test_class = "com.google.devtools.kythe.extractors.shared.ExtractorUtilsTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:junit_junit",
    ],
)
