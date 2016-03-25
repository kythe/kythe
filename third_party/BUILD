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
        "//external:gflags/license",
        "//external:glog/license",
        "//external:googletest/license",
        "//external:guava/license",
        "//external:re2/license",
        "//third_party/bazel:license",
        "//third_party/go:licenses",
        "//third_party/grpc-java:license",
        "//third_party/grpc:license",
        "//third_party/gson:license",
        "//third_party/hpack:license",
        "//third_party/jcommander:license",
        "//third_party/jsr305_annotations:license",
        "//third_party/leiningen:license",
        "//third_party/leveldb:license",
        "//third_party/llvm:license",
        "//third_party/netty:license",
        "//third_party/proto:license",
        "//third_party/rapidjson:license",
        "//third_party/re2j:license",
        "//third_party/snappy:license",
        "//third_party/zlib:license",
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
    name = "libcurl",
    srcs = ["empty.cc"],
    linkopts = ["-lcurl"],
)
