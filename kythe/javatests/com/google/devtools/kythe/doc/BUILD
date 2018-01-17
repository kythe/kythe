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
        "@com_google_common_html_types//jar",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_truth_truth//jar",
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
        "@com_google_common_html_types//jar",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)
