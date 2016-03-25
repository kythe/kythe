package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "util",
    srcs = [
        "DeleteRecursively.java",
        "KytheURI.java",
        "Span.java",
    ],
    deps = [
        "//external:guava",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
    ],
)

java_library(
    name = "json",
    srcs = [
        "JsonUtil.java",
    ],
    deps = [
        "//external:guava",
        "//third_party/gson",
        "//third_party/gson:proto",
        "//third_party/proto:protobuf_java",
    ],
)
