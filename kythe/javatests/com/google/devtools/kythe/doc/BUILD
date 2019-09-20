load("@rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "doc_unbracketer_test",
    size = "small",
    srcs = ["DocUnbracketerTest.java"],
    test_class = "com.google.devtools.kythe.doc.DocUnbracketerTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/doc:doc_unbracketer",
        "//kythe/proto:common_java_proto",
        "//kythe/proto:xref_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_common_html_types//jar",
        "@com_google_protobuf//:protobuf_java",
        "@junit_junit//jar",
    ],
)

java_test(
    name = "marked_source_renderer_test",
    size = "small",
    srcs = ["MarkedSourceRendererTest.java"],
    data = [
        "testdata/marked_source_renderer_test.textproto",
        "testdata/marked_source_signature_test.textproto",
    ],
    test_class = "com.google.devtools.kythe.doc.MarkedSourceRendererTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/doc:marked_source_renderer",
        "//kythe/proto:common_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_common_html_types//jar",
        "@com_google_protobuf//:protobuf_java",
        "@junit_junit//jar",
    ],
)
