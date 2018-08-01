package(default_visibility = ["//kythe:default_visibility"])

java_test(
    name = "options_utils_test",
    size = "small",
    srcs = ["OptionsTest.java"],
    data = [
        "//third_party/javac:java_compiler_jar",
        "//third_party/javac:jdk_compiler_jar",
    ],
    jvm_flags = [
        "--add-opens=jdk.compiler/com.sun.tools.javac.api=java.compiler",
        "--patch-module=java.compiler=$${RUNPATH}$(location //third_party/javac:java_compiler_jar)",
        "--patch-module=jdk.compiler=$${RUNPATH}$(location //third_party/javac:jdk_compiler_jar)",
    ],
    test_class = "com.google.devtools.kythe.platform.java.OptionsTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/platform/java:options",
        "//third_party/guava",
        "//third_party/javac",
        "//third_party/truth",
        "@junit_junit//jar",
    ],
)
