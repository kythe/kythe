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
        "//external:proto/protobuf_java",
        "//kythe/proto:storage_proto_java",
    ],
)

java_library(
    name = "json",
    srcs = [
        "JsonUtil.java",
    ],
    deps = [
        "//external:gson",
        "//external:gson/proto",
        "//external:guava",
        "//external:proto/protobuf_java",
    ],
)
