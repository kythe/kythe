package(default_visibility = ["//visibility:public"])

filegroup(
    name = "protoc",
    srcs = ["protobuf/bin/protoc"],
)

java_import(
    name = "protobuf",
    jars = ["protobuf/protobuf-java-3.0.0-alpha-1.jar"],
)

java_import(
    name = "junit4",
    jars = [
        "hamcrest_core/hamcrest-core-1.3.jar",
        "junit/junit-4.11.jar",
    ],
)
