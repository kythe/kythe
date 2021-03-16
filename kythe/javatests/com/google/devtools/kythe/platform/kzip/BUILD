load("@rules_java//java:defs.bzl", "java_test")

java_test(
    name = "kzip_reader_test",
    srcs = ["KZipReaderTest.java"],
    data = [
        "//kythe/testdata/platform:empty.kzip",
        "//kythe/testdata/platform:garbage_unit.kzip",
        "//kythe/testdata/platform:malformed.kzip",
        "//kythe/testdata/platform:missing-pbunit.kzip",
        "//kythe/testdata/platform:missing-unit.kzip",
        "//kythe/testdata/platform:stringset.kzip",
        "//kythe/testdata/platform:stringset_with_empty_file.kzip",
    ],
    test_class = "com.google.devtools.kythe.platform.kzip.KZipReaderTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/kzip",
        "//kythe/java/com/google/devtools/kythe/platform/shared:test_util",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:go_java_proto",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java_util",
        "@maven//:com_google_code_gson_gson",
        "@maven//:com_google_guava_guava",
        "@maven//:junit_junit",
    ],
)

java_test(
    name = "kzip_writer_test",
    srcs = ["KZipWriterTest.java"],
    data = ["//kythe/testdata/platform:stringset.kzip"],
    test_class = "com.google.devtools.kythe.platform.kzip.KZipWriterTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/kzip",
        "//kythe/java/com/google/devtools/kythe/platform/shared:test_util",
        "//kythe/java/com/google/devtools/kythe/util:json",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:go_java_proto",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java_util",
        "@maven//:junit_junit",
    ],
)
