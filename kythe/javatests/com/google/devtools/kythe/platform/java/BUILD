package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "options_utils_test",
    size = "small",
    srcs = ["OptionsTest.java"],
    data = ["//third_party/javac:javac_jar"],
    jvm_flags = ["-Xbootclasspath/p:$${RUNPATH}$(location //third_party/javac:javac_jar)"],
    test_class = "com.google.devtools.kythe.platform.java.OptionsTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java:options",
        "//third_party/javac",
        "@com_google_guava_guava//jar",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)
