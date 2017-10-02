package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "source_builder_test",
    size = "small",
    srcs = ["SourceBuilderTest.java"],
    test_class = "com.google.devtools.kythe.util.SourceBuilderTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/java/com/google/devtools/kythe/util:source_builder",
        "//kythe/proto:internal_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)

java_test(
    name = "kythe_uri_test",
    size = "small",
    srcs = ["KytheURITest.java"],
    test_class = "com.google.devtools.kythe.util.KytheURITest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)

java_test(
    name = "position_mappings_test",
    size = "small",
    srcs = ["PositionMappingsTest.java"],
    test_class = "com.google.devtools.kythe.util.PositionMappingsTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:position_mappings",
        "@com_google_guava_guava//jar",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)

java_test(
    name = "qualified_name_extractor_test",
    size = "small",
    srcs = ["QualifiedNameExtractorTest.java"],
    test_class = "com.google.devtools.kythe.util.QualifiedNameExtractorTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:qualified_name_extractor",
        "//kythe/proto:common_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)
