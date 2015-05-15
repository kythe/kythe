package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # BSD

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

cc_binary(
    name = "plugin",
    srcs = [
        "compiler/src/java_plugin/cpp/java_generator.cpp",
        "compiler/src/java_plugin/cpp/java_generator.h",
        "compiler/src/java_plugin/cpp/java_plugin.cpp",
    ],
    deps = [
        "//third_party/proto:libprotoc",
    ],
)

java_import(
    name = "grpc-java",
    jars = [
        "grpc-all-0.1.0-SNAPSHOT.jar",
    ],
)
