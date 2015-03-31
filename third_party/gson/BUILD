package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # Authored by Google, Apache 2.0

exports_files(["LICENSE"])

java_import(
    name = "gson",
    jars = [
        "gson-2.3-SNAPSHOT.jar",
    ],
)

java_library(
    name = "proto",
    srcs = [
        "ProtoTypeAdapter.java",
    ],
    deps = [
        ":gson",
        "//third_party:protobuf",
    ],
)
