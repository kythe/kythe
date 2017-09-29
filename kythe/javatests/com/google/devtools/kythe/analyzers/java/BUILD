sh_test(
    name = "indexer_test",
    size = "small",
    srcs = ["indexer_test.sh"],
    data = [
        "//kythe/go/platform/tools/entrystream",
        "//kythe/go/platform/tools/indexpack",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
        "//kythe/testdata:test.kindex",
    ],
)

sh_test(
    name = "indexer_empty_cu_test",
    size = "small",
    srcs = ["indexer_empty_cu_test.sh"],
    data = [
        "//kythe/go/platform/tools/entrystream",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
        "//kythe/testdata:java_empty.kindex",
    ],
)

# See https://kythe.io/phabricator/T70
sh_test(
    name = "indexer_source_root_test",
    srcs = ["indexer_source_root_test.sh"],
    data = [
        "//kythe/go/platform/tools/entrystream",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata:corner_case.kindex",

        # Having the following sources in the Java indexer's source path will cause T70.  The
        # indexer reads the .java file instead of the .class file found in corner_case.kindex,
        # causing javac to compile these sources without their depedencies (the transitive
        # dependencies of corner_case.kindex's compilation).
        "//kythe/java/com/google/devtools/kythe/analyzers/base:corner_case_srcs",
    ],
    tags = [
        "broken",
        "manual",
    ],
)

java_test(
    name = "mini_anchor_test",
    size = "small",
    srcs = ["MiniAnchorTest.java"],
    test_class = "com.google.devtools.kythe.analyzers.java.MiniAnchorTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/java:analyzer",
        "@com_google_guava_guava//jar",
        "@com_google_truth_truth//jar",
        "@junit_junit//jar",
    ],
)

java_library(
    name = "test_plugin",
    srcs = ["TestPlugin.java"],
    visibility = ["//kythe/javatests/com/google/devtools/kythe/analyzers/java:__subpackages__"],
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/base:entryset",
        "//kythe/java/com/google/devtools/kythe/analyzers/java:plugin",
        "//kythe/java/com/google/devtools/kythe/common:autoservice",
        "//kythe/proto:storage_proto_java",
        "@bazel_tools//third_party/java/jdk/langtools:javac",
    ],
)

# java_analyzer_fake_binary is so that these tests are processed as part of the
# regular Kythe pipeline, so that we can do end-to-end tests for tools that use
# Kythe facts.

java_library(
    name = "java_analyzer_fake_library",
    srcs = glob(
        ["testdata/**/*.java"],
        exclude = [
            # Exclude meta-data tests
            "testdata/pkg/Metadata.java",
            "testdata/pkg/ProtobufMetadata.java",
        ],
    ),
)
