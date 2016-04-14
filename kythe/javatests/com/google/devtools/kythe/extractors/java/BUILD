java_test(
    name = "java_extractor_test",
    srcs = ["JavaExtractorTest.java"],
    data = glob(["testdata/**"]) + [
        ":SillyProcessor_deploy.jar",
        "@bazel_tools//third_party/java/jdk/langtools:javac_jar",
    ],
    javacopts = ["-Xlint:all"],
    jvm_flags = [
        # TODO(shahms): Pick one of these once Bazel figures it out.
        #   Currently (0.2.1) Bazel is inconsistent in whether or not the runfiles paths
        #   include the workspace name.  Specifically, they do on Linux but not on OS X.
        "-Xbootclasspath/p:$$JAVA_RUNFILES/$(location @bazel_tools//third_party/java/jdk/langtools:javac_jar)",
        "-Xbootclasspath/p:$$JAVA_RUNFILES/io_kythe/$(location @bazel_tools//third_party/java/jdk/langtools:javac_jar)",
    ],
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
