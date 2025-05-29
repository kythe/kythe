package(default_visibility = ["//visibility:public"])

# This filegroup should include all licenses of third_party libraries
# distributed in Kythe binary releases.
# Note: many of the licenses are vendored for external repo deps; double-check
# %workspace%/external.bzl to ensure that any license deleted is actually
# unused.
filegroup(
    name = "licenses",
    srcs = ["README"] + [
        "//third_party/absl:LICENSE",
        "//third_party/auto:LICENSE.txt",
        "//third_party/bazel:license",
        "//third_party/boringssl:LICENSE",
        "//third_party/errorprone:COPYING",
        "//third_party/gflags:COPYING.txt",
        "//third_party/glog:COPYING",
        "//third_party/go:licenses",
        "//third_party/guava:COPYING",
        "//third_party/jcommander:license.txt",
        "//third_party/jsr250_api:LICENSE",
        "//third_party/jsr305:LICENSE",
        "//third_party/libmemcached:COPYING",
        "//third_party/libuuid:COPYING",
        "//third_party/llvm:license",
        "//third_party/ow2-asm:LICENSE.txt",
        "//third_party/re2:LICENSE",
        "//third_party/re2j:LICENSE",
        "//third_party/riegeli:LICENSE",
        "//third_party/safe_html_types:LICENSE",
        "//third_party/truth:LICENSE",
        "//third_party/zlib:README",
        "@com_github_google_leveldb//:license",
        "@com_github_google_snappy//:license",
        "@com_github_tencent_rapidjson//:license",
        "@com_google_protobuf//:LICENSE",
        "@org_brotli//:LICENSE",
    ],
)

cc_library(
    name = "libuuid",
    srcs = ["empty.cc"],
    linkopts = select({
        "@bazel_tools//src/conditions:darwin": [],
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

alias(
    name = "go_sdk_files",
    actual = select({
        "@io_bazel_rules_go//go/platform:darwin":  "@go_sdk_darwin_arm64//:files",
        "//conditions:default": "@go_sdk_linux//:files",
    }),
    visibility = ["//visibility:public"],
)
