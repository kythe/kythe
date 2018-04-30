java_test(
    name = "java_extractor_test",
    size = "small",
    srcs = ["JavaExtractorTest.java"],
    data = glob(["testdata/**"]) + [
        ":SillyProcessor_deploy.jar",
        "//third_party/javac:javac_jar",
    ],
    javacopts = ["-Xlint:all"],
    jvm_flags = [
        "-Xbootclasspath/p:$${RUNPATH}$(location //third_party/javac:javac_jar)",
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
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:protobuf_java",
        "@com_google_truth_truth//jar",
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
    deps = ["@bazel_tools//tools/jdk:langtools-neverlink"],
)
