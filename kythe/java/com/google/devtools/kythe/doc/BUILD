load("@rules_java//java:defs.bzl", "java_library")

package(default_visibility = ["//kythe:default_visibility"])

java_library(
    name = "doc_unbracketer",
    srcs = ["DocUnbracketer.java"],
    deps = [
        "//kythe/proto:common_java_proto",
        "//kythe/proto:xref_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_common_html_types_types",
    ],
)

java_library(
    name = "marked_source_renderer",
    srcs = ["MarkedSourceRenderer.java"],
    deps = [
        "//kythe/proto:common_java_proto",
        "//third_party/guava",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:com_google_common_html_types_types",
        "@maven//:org_checkerframework_checker_qual",
    ],
)
