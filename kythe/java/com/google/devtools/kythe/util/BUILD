package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "util",
    srcs = [
        "DeleteRecursively.java",
        "KytheURI.java",
        "Span.java",
    ],
    deps = [
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_guava_guava//jar",
    ],
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
