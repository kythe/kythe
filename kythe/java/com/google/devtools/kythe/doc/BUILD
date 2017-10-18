package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "doc_unbracketer",
    srcs = ["DocUnbracketer.java"],
    deps = [
        "//kythe/proto:common_proto_java",
        "//kythe/proto:xref_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_common_html_types//jar",
        "@com_google_guava_guava//jar",
    ],
)

java_library(
    name = "marked_source_renderer",
    srcs = ["MarkedSourceRenderer.java"],
    deps = [
        "//kythe/proto:common_proto_java",
        "//third_party/proto:protobuf_java",
        "@com_google_common_html_types//jar",
        "@com_google_guava_guava//jar",
    ],
)
