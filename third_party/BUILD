package(default_visibility = ["//visibility:public"])

filegroup(
    name = "srcs",
    srcs = glob(["**"]),
    visibility = ["//visibility:private"],
)

# This filegroup should include all licenses of third_party libraries
# distributed in Kythe binary releases.
filegroup(
    name = "licenses",
    srcs = ["README"] + [
        "//third_party/go:licenses",
        "//third_party/googleflags:license",
        "//third_party/googlelog:license",
        "//third_party/googletest:license",
        "//third_party/grpc:license",
        "//third_party/grpc-java:license",
        "//third_party/gson:license",
        "//third_party/guava:license",
        "//third_party/jsr305_annotations:license",
        "//third_party/leveldb:license",
        "//third_party/llvm:license",
        "//third_party/netty:license",
        "//third_party/openjdk:license",
        "//third_party/leiningen:license",
        "//third_party/proto:license",
        "//third_party/re2:license",
        "//third_party/rapidjson:license",
        "//third_party/snappy:license",
        "//third_party/zlib:license",
        "//third_party/re2j:license",
    ],
)

java_import(
    name = "junit4",
    jars = [
        "hamcrest_core/hamcrest-core-1.3.jar",
        "junit/junit-4.11.jar",
    ],
)

cc_library(
    name = "libuuid",
    srcs = ["empty.cc"],
    linkopts = ["-luuid"],
)

cc_library(
    name = "libcurl",
    srcs = ["empty.cc"],
    linkopts = ["-lcurl"],
)

cc_library(
    name = "libcrypto",
    srcs = ["empty.cc"],
    linkopts = ["-lcrypto"],
)
