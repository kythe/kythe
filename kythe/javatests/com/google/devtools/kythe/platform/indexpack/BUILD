package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "archive_test",
    size = "small",
    srcs = ["ArchiveTest.java"],
    test_class = "com.google.devtools.kythe.platform.indexpack.ArchiveTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)
