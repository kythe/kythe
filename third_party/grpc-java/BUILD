package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # BSD

exports_files(["LICENSE"])

filegroup(
    name = "plugin",
    srcs = ["java_plugin"],
)

java_import(
    name = "grpc-java",
    jars = [
        "grpc-all-0.1.0-SNAPSHOT.jar",
    ],
)
