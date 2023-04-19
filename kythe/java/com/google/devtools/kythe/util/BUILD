load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "source_builder",
    srcs = ["SourceBuilder.java"],
    deps = [
        ":kythe_uri",
        "//kythe/proto:internal_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "normalizer",
    srcs = ["Normalizer.java"],
    visibility = ["//visibility:public"],
    deps = [
        "//kythe/proto:common_java_proto",
        "//kythe/proto:serving_java_proto",
        "@com_google_protobuf//:protobuf_java",
    ],
)

# N.B. This package is given proto visibility although it is not a proto,
# because it is impractical to use the Kythe protos without ticket support.
java_library(
    name = "kythe_uri",
    srcs = ["KytheURI.java"],
    visibility = ["//kythe:proto_visibility"],
    deps = [
        "//kythe/proto:common_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "span",
    srcs = ["Span.java"],
    visibility = ["//visibility:public"],
)

java_library(
    name = "delete_recursively",
    srcs = ["DeleteRecursively.java"],
)

java_library(
    name = "json",
    srcs = ["JsonUtil.java"],
    deps = [
        "//kythe/proto:buildinfo_java_proto",
        "//kythe/proto:java_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_protobuf//:protobuf_java_util",
        "@maven//:com_google_code_gson_gson",
        "@maven//:org_checkerframework_checker_qual",
    ],
)

java_library(
    name = "position_mappings",
    srcs = ["PositionMappings.java"],
    visibility = ["//visibility:public"],
)

java_library(
    name = "qualified_name_extractor",
    srcs = ["QualifiedNameExtractor.java"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/doc:marked_source_renderer",
        "//kythe/proto:common_java_proto",
    ],
)

java_library(
    name = "ordered_compatibility_service",
    srcs = ["OrderedCompatibilityService.java"],
    deps = [
        "//third_party/guava",
    ],
)

# Sources used to test Java extraction in //kythe/release:release_test.
filegroup(
    name = "test_srcs",
    srcs = [
        "DeleteRecursively.java",
        "PositionMappings.java",
        "Span.java",
    ],
)
