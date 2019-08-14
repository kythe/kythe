load("@rules_java//java:defs.bzl", "java_test")

java_test(
    name = "stream_fact_emitter_test",
    size = "small",
    srcs = ["StreamFactEmitterTest.java"],
    test_class = "com.google.devtools.kythe.analyzers.base.StreamFactEmitterTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:stream_fact_emitter",
        "//kythe/proto:storage_java_proto",
        "//third_party/truth",
        "@junit_junit//jar",
    ],
)
