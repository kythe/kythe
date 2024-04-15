load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load("//tools/build_rules/extra_aspects:config.bzl", "kythe_extractor_config")
load(":config.bzl", "cpp_compile_info_proto_writer")

kythe_extractor_config(
    name = "extractor-config",
    action_listeners = [
        "//kythe/extractors:extract_kzip_cxx",
    ],
    aspect_rule_attrs = {"cc_proto_library": []},
    proto_writer = ":cpp-compile-info-writer",
    rules = [
        "cc_binary",
        "cc_library",
        "cc_test",
        "cc_stubby_library",
        "py_extension",
        "proto_library",
    ],
    visibility = ["//visibility:public"],
)

cpp_compile_info_proto_writer(name = "cpp-compile-info-writer")

bzl_library(
    name = "config",
    srcs = ["config.bzl"],
    deps = ["//tools/build_rules/extra_aspects:extra_actions"],
)

bzl_library(
    name = "aspect",
    srcs = ["aspect.bzl"],
    deps = [
        "//tools/build_rules/extra_aspects:config",
        "@rules_proto//proto:defs",
    ],
)

bzl_library(
    name = "rules",
    srcs = ["rules.bzl"],
    visibility = ["//visibility:public"],
    deps = [
        ":aspect",
    ],
)
