load("@rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "source_builder_test",
    size = "small",
    srcs = ["SourceBuilderTest.java"],
    test_class = "com.google.devtools.kythe.util.SourceBuilderTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:source_builder",
        "//kythe/proto:internal_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:junit_junit",
    ],
)

java_test(
    name = "point_expander_test",
    size = "small",
    srcs = ["NormalizerTest.java"],
    test_class = "com.google.devtools.kythe.util.NormalizerTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:normalizer",
        "//kythe/proto:common_java_proto",
        "//kythe/proto:serving_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:junit_junit",
    ],
)

java_test(
    name = "kythe_uri_test",
    size = "small",
    srcs = ["KytheURITest.java"],
    test_class = "com.google.devtools.kythe.util.KytheURITest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:kythe_uri",
        "//kythe/proto:storage_java_proto",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:junit_junit",
    ],
)

java_test(
    name = "position_mappings_test",
    size = "small",
    srcs = ["PositionMappingsTest.java"],
    test_class = "com.google.devtools.kythe.util.PositionMappingsTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:position_mappings",
        "//third_party/guava",
        "//third_party/truth",
        "@maven//:junit_junit",
    ],
)

java_test(
    name = "qualified_name_extractor_test",
    size = "small",
    srcs = ["QualifiedNameExtractorTest.java"],
    test_class = "com.google.devtools.kythe.util.QualifiedNameExtractorTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util:qualified_name_extractor",
        "//kythe/proto:common_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:junit_junit",
    ],
)
