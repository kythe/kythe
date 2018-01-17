package(default_visibility = ["//visibility:public"])

licenses(["unencumbered"])  # Authored by Google, Apache 2.0

filegroup(
    name = "license",
    srcs = ["LICENSE"],
)

java_library(
    name = "proto",
    srcs = [
        "ProtoTypeAdapter.java",
    ],
    deps = [
        "@com_google_code_gson_gson//jar",
        "@com_google_guava_guava//jar",
        "@com_google_protobuf//:protobuf_java",
    ],
)
