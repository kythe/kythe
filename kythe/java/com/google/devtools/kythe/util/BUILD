package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "source_builder",
    srcs = ["SourceBuilder.java"],
    deps = [
        ":kythe_uri",
        "//kythe/proto:internal_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
    ],
)

java_library(
    name = "normalizer",
    srcs = ["Normalizer.java"],
    deps = [
        "//kythe/proto:common_proto_java",
        "//kythe/proto:serving_proto_java",
        "//third_party/proto:protobuf_java",
    ],
)

java_library(
    name = "kythe_uri",
    srcs = ["KytheURI.java"],
    deps = [
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
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
        "//third_party/proto:protobuf_java",
        "//third_party/proto:protobuf_java_util",
        "@com_google_code_gson_gson//jar",
        "@com_google_guava_guava//jar",
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
        "//kythe/proto:common_proto_java",
    ],
)
