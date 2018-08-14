package(default_visibility = ["//visibility:public"])

licenses(["restricted"])  # GNU GPL v2 with Classpath exception

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

java_import(
    name = "javac",
    jars = [
        "jdk_compiler.jar",
        "java_compiler.jar",
    ],
)

filegroup(
    name = "jdk_compiler_jar",
    srcs = ["jdk_compiler.jar"],
)

filegroup(
    name = "java_compiler_jar",
    srcs = ["java_compiler.jar"],
)
