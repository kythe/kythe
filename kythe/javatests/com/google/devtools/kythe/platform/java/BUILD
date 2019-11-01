load("@rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "options_utils_test",
    size = "small",
    srcs = ["OptionsTest.java"],
    test_class = "com.google.devtools.kythe.platform.java.OptionsTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java:options",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:java_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "//third_party/truth",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@maven//:junit_junit",
    ],
)
