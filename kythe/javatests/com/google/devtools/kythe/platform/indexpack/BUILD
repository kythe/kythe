package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "archive_test",
    size = "small",
    srcs = ["ArchiveTest.java"],
    test_class = "com.google.devtools.kythe.platform.indexpack.ArchiveTest",
    deps = [
        "//external:guava",
        "//external:junit4",
        "//external:proto/protobuf_java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/java/com/google/devtools/kythe/platform/indexpack",
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
    ],
)
