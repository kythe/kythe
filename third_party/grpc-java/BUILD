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
    deps = ["//third_party/proto:libprotoc"],
)

java_library(
    name = "grpc-java",
    srcs = ["grpc-all-0.9.0.jar"],
    runtime_deps = [
        "//third_party/hpack",
        "//third_party/netty",
    ],
)
