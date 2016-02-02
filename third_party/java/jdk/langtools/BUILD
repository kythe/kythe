package(default_visibility = ["//visibility:public"])

licenses(["restricted"])  # GNU GPL v2 with Classpath exception

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
)

java_import(
    name = "javac",
    jars = [":javac_jar"],
)

java_library(
    name = "javac-neverlink",
    neverlink = 1,
    exports = [":javac"],
)

filegroup(
    name = "javac_jar",
    srcs = [
        "@bazel_tools//tools/jdk:langtools-neverlink",
    ],
)
