load("@rules_java//java:defs.bzl", "java_library", "java_test")
load("//tools:build_rules/testing.bzl", "shell_tool_test")

shell_tool_test(
    name = "indexer_test",
    data = ["//kythe/testdata:test.kindex"],
    scriptfile = "indexer_test.sh",
    tools = {
        "entrystream": "//kythe/go/platform/tools/entrystream",
        "indexer": "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
        "indexpack": "//kythe/go/platform/tools/indexpack",
    },
)

shell_tool_test(
    name = "indexer_empty_cu_test",
    data = ["//kythe/testdata:java_empty.kindex"],
    scriptfile = "indexer_empty_cu_test.sh",
    tools = {
        "entrystream": "//kythe/go/platform/tools/entrystream",
        "indexer": "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
    },
)

# See #818
shell_tool_test(
    name = "indexer_source_root_test",
    data = [
        "//kythe/javatests/com/google/devtools/kythe/analyzers/java/testdata:corner_case.kindex",
        # Having the following sources in the Java indexer's source path caused #818.  The
        # indexer read the .java file instead of the .class file found in corner_case.kindex,
        # causing javac to compile these sources without their depedencies (the transitive
        # dependencies of corner_case.kindex's compilation).
        "//kythe/java/com/google/devtools/kythe/analyzers/base:corner_case_srcs",
    ],
    scriptfile = "indexer_source_root_test.sh",
    tools = {
        "entrystream": "//kythe/go/platform/tools/entrystream",
        "indexer": "//kythe/java/com/google/devtools/kythe/analyzers/java:indexer",
    },
)

java_test(
    name = "mini_anchor_test",
    size = "small",
    srcs = ["MiniAnchorTest.java"],
    test_class = "com.google.devtools.kythe.analyzers.java.MiniAnchorTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/java:analyzer",
        "//third_party/guava",
        "//third_party/truth",
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
        "//kythe/proto:storage_java_proto",
        "//third_party/javac",
    ],
)
