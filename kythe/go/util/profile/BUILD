load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "profile",
    srcs = ["profile.go"],
    importpath = "kythe.io/kythe/go/util/profile",
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
    ],
)
