java_test(
    name = "java_extractor_test",
    size = "small",
    srcs = ["JavaExtractorTest.java"],
    data = glob(["testdata/**"]) + [
        ":SillyProcessor_deploy.jar",
        "//third_party/javac:java_compiler_jar",
        "//third_party/javac:jdk_compiler_jar",
    ],
    javacopts = ["-Xlint:all"],
    jvm_flags = [
        "--add-opens=jdk.compiler/com.sun.tools.javac.api=java.compiler",
        "--patch-module=java.compiler=$${RUNPATH}$(location //third_party/javac:java_compiler_jar)",
        "--patch-module=jdk.compiler=$${RUNPATH}$(location //third_party/javac:jdk_compiler_jar)",
    ],
    test_class = "com.google.devtools.kythe.extractors.java.JavaExtractorTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/proto:analysis_java_proto",
        "//kythe/proto:java_java_proto",
        "//kythe/proto:storage_java_proto",
        "//third_party/guava",
        "//third_party/javac",
        "//third_party/truth",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@junit_junit//jar",
    ],
)

java_binary(
    name = "SillyProcessor",
    srcs = [
        "testdata/processor/Silly.java",
        "testdata/processor/SillyProcessor.java",
    ],
    create_executable = 0,
    deps = ["//third_party/javac"],
)
