package(default_visibility = ["//visibility:public"])

# This filegroup should include all licenses of third_party libraries
# distributed in Kythe binary releases.
filegroup(
    name = "licenses",
    srcs = ["README"] + [
        "//third_party/bazel:license",
        "//third_party/go:licenses",
        "//third_party/grpc-java:license",
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

exports_files(["libmemcached.mem_config.h"])

alias(
    name = "gtest",
    actual = "@com_github_google_googletest//:gtest",
)

alias(
    name = "gtest_main",
    actual = "@com_github_google_googletest//:gtest_main",
)

alias(
    name = "gmock",
    actual = "@com_github_google_googletest//:gmock",
)

alias(
    name = "gmock_main",
    actual = "@com_github_google_googletest//:gmock_main",
)
