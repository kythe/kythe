java_test(
    name = "java_extractor_test",
    srcs = ["JavaExtractorTest.java"],
    data = glob(["testdata/**"]) + [
        ":SillyProcessor_deploy.jar",
        "//third_party/java/jdk/langtools:javac_jar",
    ],
    javacopts = ["-Xlint:all"],
    jvm_flags = ["-Xbootclasspath/p:$$JAVA_RUNFILES/external/bazel_tools/third_party/java/jdk/langtools/javac.jar"],
    test_class = "com.google.devtools.kythe.extractors.java.JavaExtractorTest",
    deps = [
        "//external:guava",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:java_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party:junit4",
        "//third_party/java/jdk/langtools:javac",
        "//third_party/proto:protobuf_java",
        "//third_party/truth",
    ],
)

java_binary(
    name = "SillyProcessor",
    srcs = [
        "testdata/processor/Silly.java",
        "testdata/processor/SillyProcessor.java",
    ],
    create_executable = 0,
    deps = ["//third_party/java/jdk/langtools:javac-neverlink"],
)
