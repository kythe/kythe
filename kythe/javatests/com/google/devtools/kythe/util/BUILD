package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "kythe_uri_test",
    size = "small",
    srcs = ["KytheURITest.java"],
    test_class = "com.google.devtools.kythe.util.KytheURITest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/util",
        "//kythe/proto:storage_proto_java",
        "//third_party/proto:protobuf_java",
        "@junit_junit//jar",
    ],
)
