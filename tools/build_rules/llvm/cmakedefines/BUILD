load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

go_library(
    name = "go_default_library",
    srcs = ["cmakedefines.go"],
    importpath = "github.com/shahms/cmaketobzl/tools/cmakedefines",
    visibility = ["//visibility:private"],
)

go_binary(
    name = "cmakedefines",
    embed = [":go_default_library"],
    visibility = ["//visibility:public"],
)
