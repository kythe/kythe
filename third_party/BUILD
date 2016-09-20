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
        "//external:gson/license",
        "//external:guava/license",
        "//external:jcommander/license",
        "//external:jsr305/license",
        "//external:junit4/license",
        "//external:netty/license",
        "//external:re2/license",
        "//external:re2j/license",
        "//external:truth/license",
        "//third_party/bazel:license",
        "//third_party/go:licenses",
        "//third_party/grpc-java:license",
        "//third_party/grpc:license",
        "//third_party/leiningen:license",
        "//third_party/leveldb:license",
        "//third_party/llvm:license",
        "//third_party/proto:license",
        "//third_party/rapidjson:license",
        "//third_party/snappy:license",
        "//third_party/zlib:license",
    ],
)

cc_library(
    name = "libcurl",
    srcs = ["empty.cc"],
    linkopts = ["-lcurl"],
)
