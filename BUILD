package(default_visibility = ["//visibility:private"])

load("//:version.bzl", "MAX_VERSION", "MIN_VERSION")
load("@bazel_gazelle//:def.bzl", "gazelle")

exports_files(glob(["*"]))

filegroup(
    name = "nothing",
    visibility = ["//visibility:public"],
)

config_setting(
    name = "darwin",
    values = {"cpu": "darwin"},
    visibility = ["//visibility:public"],
)

sh_test(
    name = "check_bazel_versions",
    srcs = ["//tools:check_bazel_versions.sh"],
    args = [
        MIN_VERSION,
        MAX_VERSION,
    ],
    data = [
        ".bazelminversion",
        ".bazelversion",
    ],
)

# gazelle:build_file_name BUILD
# gazelle:exclude kythe
# gazelle:exclude third_party
# gazelle:prefix kythe.io
gazelle(name = "gazelle")
