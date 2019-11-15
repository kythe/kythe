load("@rules_java//java:defs.bzl", "java_test")

java_test(
    name = "jvm_graph_test",
    size = "small",
    srcs = ["JvmGraphTest.java"],
    test_class = "com.google.devtools.kythe.analyzers.jvm.JvmGraphTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/jvm:jvm_graph",
        "//third_party/guava",
        "//third_party/truth",
        "@maven//:junit_junit",
    ],
)
