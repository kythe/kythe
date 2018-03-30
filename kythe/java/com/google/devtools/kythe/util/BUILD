package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "source_builder",
    srcs = ["SourceBuilder.java"],
    deps = [
        ":kythe_uri",
        "//kythe/proto:internal_java_proto",
        "//kythe/proto:storage_java_proto",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "normalizer",
    srcs = ["Normalizer.java"],
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
        "//kythe/proto:storage_java_proto",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)

java_library(
    name = "span",
    srcs = ["Span.java"],
)

java_library(
    name = "delete_recursively",
    srcs = ["DeleteRecursively.java"],
)

java_library(
    name = "json",
    srcs = ["JsonUtil.java"],
    deps = [
        "//third_party/gson:proto",
        "@com_google_code_gson_gson//jar",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_protobuf//:protobuf_java_util",
    ],
)

java_library(
    name = "position_mappings",
    srcs = ["PositionMappings.java"],
)

java_library(
    name = "qualified_name_extractor",
    srcs = ["QualifiedNameExtractor.java"],
    deps = [
        "//kythe/proto:common_java_proto",
    ],
)
