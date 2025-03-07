load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kvutil",
    srcs = ["kvutil.go"],
    importpath = "kythe.io/kythe/go/storage/kvutil",
    deps = [
        "//kythe/go/storage/keyvalue",
        "//kythe/go/util/log",
    ],
)
