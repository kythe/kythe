load("@io_bazel_rules_java//java:defs.bzl", "java_import")

package(default_visibility = ["//visibility:public"])

licenses(["restricted"])  # GNU GPL v2 with Classpath exception

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

java_import(
    name = "javac",
    jars = [":javac_jar"],
    neverlink = True,
)

java_import(
    name = "javac9",
    jars = [
        ":jdk_compiler_jar",
        ":java_compiler_jar",
    ],
    neverlink = True,
)

filegroup(
    name = "javac_jar",
    srcs = ["javac-9+181-r4173-1.jar"],
)

filegroup(
    name = "jdk_compiler_jar",
    srcs = ["jdk_compiler.jar"],
)

filegroup(
    name = "java_compiler_jar",
    srcs = ["java_compiler.jar"],
)
