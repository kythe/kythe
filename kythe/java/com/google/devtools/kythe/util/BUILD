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
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)

java_library(
    name = "json",
    srcs = [
        "JsonUtil.java",
    ],
    deps = [
        "//third_party/gson",
        "//third_party/gson:proto",
        "//third_party/guava",
        "//third_party/proto:protobuf_java",
    ],
)
