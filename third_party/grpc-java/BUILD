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
        "@io_grpc_grpc_core//jar:file",
        "@io_grpc_grpc_context//jar:file",
        "@io_grpc_grpc_netty//jar:file",
        "@io_grpc_grpc_protobuf//jar:file",
        "@io_grpc_grpc_protobuf_lite//jar:file",
        "@io_grpc_grpc_stub//jar:file",
    ],
    runtime_deps = [
        "@io_netty_netty_all//jar",
    ],
)
