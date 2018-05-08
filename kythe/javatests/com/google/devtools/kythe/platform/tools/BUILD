package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "mvn_pom_preprocessor_test",
    size = "small",
    srcs = ["MvnPomPreprocessorTest.java"],
    data = [
        "testdata/test_pom_with_plugin.xml",
        "testdata/test_pom_without_plugin.xml",
    ],
    test_class = "com.google.devtools.kythe.platform.tools.MvnPomPreprocessorTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/tools:mvn_pom_preprocessor",
        "@com_google_guava_guava//jar",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)
