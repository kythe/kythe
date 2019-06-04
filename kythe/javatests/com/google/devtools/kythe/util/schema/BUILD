package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "nodes_test",
    size = "small",
    srcs = ["NodesTest.java"],
    test_class = "com.google.devtools.kythe.util.schema.NodesTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util/schema",
        "//kythe/proto:internal_java_proto",
        "//kythe/proto:schema_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@com_google_protobuf//:protobuf_java",
        "@junit_junit//jar",
    ],
)
