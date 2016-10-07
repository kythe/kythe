package(default_visibility = ["//visibility:private"])

load("@io_bazel_rules_go//go:def.bzl", "go_prefix")

exports_files(glob(["*"]))

go_prefix("kythe.io/")

filegroup(
    name = "nothing",
    visibility = ["//visibility:public"],
)

config_setting(
    name = "darwin",
    values = {"cpu": "darwin"},
    visibility = ["//visibility:public"],
)
