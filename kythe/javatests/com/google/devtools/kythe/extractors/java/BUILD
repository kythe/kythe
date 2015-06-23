java_test(
    name = "java_extractor_test",
    srcs = ["JavaExtractorTest.java"],
    data = glob(["testdata/**"]) + [":SillyProcessor_deploy.jar"],
    javacopts = ["-Xlint:all"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/common",
        "//kythe/java/com/google/devtools/kythe/extractors/java",
        "//kythe/java/com/google/devtools/kythe/extractors/shared",
        "//kythe/proto:analysis_proto_java",
        "//kythe/proto:storage_proto_java",
        "//third_party:junit4",
        "//third_party/guava",
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
    deps = [
        "//tools/jdk:langtools-neverlink",
    ],
)
