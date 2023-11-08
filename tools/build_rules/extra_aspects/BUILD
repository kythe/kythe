"""Rules for Aspect-based extra actions."""

load("@bazel_skylib//:bzl_library.bzl", "bzl_library")
load(":extra_actions.bzl", "spawn_info_proto_writer")

bzl_library(
    name = "actions",
    srcs = ["actions.bzl"],
    deps = [
        "@bazel_skylib//lib:sets",
    ],
)

bzl_library(
    name = "extra_actions",
    srcs = ["extra_actions.bzl"],
    deps = [
        ":actions",
        "@bazel_skylib//lib:sets",
    ],
)

bzl_library(
    name = "config",
    srcs = ["config.bzl"],
    deps = [
        ":actions",
        ":extra_actions",
        "@bazel_skylib//lib:sets",
        "@bazel_skylib//lib:structs",
    ],
)

spawn_info_proto_writer(name = "spawn-info-writer")
