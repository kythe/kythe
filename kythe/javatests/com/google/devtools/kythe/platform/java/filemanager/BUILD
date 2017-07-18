package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "compilation_unit_file_tree_test",
    size = "small",
    srcs = ["CompilationUnitFileTreeTest.java"],
    test_class = "com.google.devtools.kythe.platform.java.filemanager.CompilationUnitFileTreeTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager",
        "//kythe/proto:analysis_proto_java",
        "@com_google_guava_guava//jar",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)
