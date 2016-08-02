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

java_import(
    name = "grpc-java",
    jars = [
        "grpc-core-1.0.0-pre1.jar",
        "grpc-stub-1.0.0-pre1.jar",
        "grpc-netty-1.0.0-pre1.jar",
        "grpc-protobuf-1.0.0-pre1.jar",
        "grpc-protobuf-lite-1.0.0-pre1.jar",
    ],
    runtime_deps = [
        "//third_party/hpack",
        "//third_party/netty",
    ],
)
