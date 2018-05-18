package(default_visibility = ["//visibility:public"])

# This filegroup should include all licenses of third_party libraries
# distributed in Kythe binary releases.
filegroup(
    name = "licenses",
    srcs = ["README"] + [
        "//third_party/bazel:license",
        "//third_party/go:licenses",
        "//third_party/gson:license",
        "//third_party/javac:license",
        "//third_party/leiningen:license",
        "//third_party/leveldb:license",
        "//third_party/llvm:license",
        "//third_party/snappy:license",
        "@com_github_tencent_rapidjson//:license",
    ],
)

cc_library(
    name = "libcurl",
    srcs = ["empty.cc"],
    linkopts = ["-lcurl"],
)

cc_library(
    name = "libuuid",
    srcs = ["empty.cc"],
    linkopts = select({
        "//:darwin": [],
        "//conditions:default": ["-luuid"],
    }),
)

exports_files(["libmemcached.mem_config.h"])

alias(
    name = "gtest",
    actual = "@com_google_googletest//:gtest",
)

alias(
    name = "gtest_main",
    actual = "@com_google_googletest//:gtest_main",
)

alias(
    name = "gmock",
    actual = "@com_google_googletest//:gtest",
)

alias(
    name = "gmock_main",
    actual = "@com_google_googletest//:gtest_main",
)
