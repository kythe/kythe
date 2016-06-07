java_test(
    name = "java_extractor_test",
    size = "small",
    srcs = ["JavaExtractorTest.java"],
    data = glob(["testdata/**"]) + [
        ":SillyProcessor_deploy.jar",
        "@bazel_tools//third_party/java/jdk/langtools:javac_jar",
    ],
    javacopts = ["-Xlint:all"],
    # TODO(schroederc): fix bootclasspath to use $(location ...) syntax
    jvm_flags = ["-Xbootclasspath/p:$$JAVA_RUNFILES/bazel_tools/third_party/java/jdk/langtools/javac.jar"],
    test_class = "com.google.devtools.kythe.extractors.java.JavaExtractorTest",
    deps = [
        "@bazel_tools//third_party/java/jdk/langtools:javac",
        "//external:guava",
        "//external:junit4",
        "//external:proto/protobuf_java",
        "//external:truth",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:java_proto_java",
        "//kythe/proto:storage_proto_java",
    ],
)

java_binary(
    name = "SillyProcessor",
    srcs = [
        "testdata/processor/Silly.java",
        "testdata/processor/SillyProcessor.java",
    ],
    create_executable = 0,
    deps = ["@bazel_tools//tools/jdk:langtools-neverlink"],
)
