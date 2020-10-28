load("@rules_java//java:defs.bzl", "java_test")

package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "compilation_unit_file_tree_test",
    size = "small",
    srcs = ["CompilationUnitFileTreeTest.java"],
    test_class = "com.google.devtools.kythe.platform.java.filemanager.CompilationUnitFileTreeTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager",
        "//kythe/java/com/google/devtools/kythe/platform/java/filemanager:filetree",
        "//kythe/proto:analysis_java_proto",
        "//third_party/guava",
        "//third_party/truth",
        "@maven//:junit_junit",
    ],
)
