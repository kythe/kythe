java_test(
    name = "label_util_test",
    size = "small",
    srcs = ["LabelUtilTest.java"],
    test_class = "com.google.devtools.kythe.analyzers.bazel.LabelUtilTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/bazel:LabelUtil",
        "@junit_junit//jar",
    ],
)

java_test(
    name = "tickets_test",
    size = "small",
    srcs = ["TicketsTest.java"],
    test_class = "com.google.devtools.kythe.analyzers.bazel.TicketsTest",
    deps = [
        "//kythe/java/com/google/devtools/kythe/analyzers/bazel:Tickets",
        "//kythe/java/com/google/devtools/kythe/util",
        "@com_google_truth_truth//jar",
        "@io_bazel//src/main/java/com/google/devtools/build/lib:skylark-lang",
        "@junit_junit//jar",
    ],
)
