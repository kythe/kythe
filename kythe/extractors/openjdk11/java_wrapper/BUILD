load("//tools:build_rules/shims.bzl", "go_binary")

go_binary(
    name = "java_wrapper",
    srcs = ["java_wrapper.go"],
    visibility = [
        "//kythe/extractors/openjdk11:__subpackages__",
    ],
    deps = [
        "//kythe/go/util/log",
        "@org_bitbucket_creachadair_shell//:shell",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_x_sys//unix",
    ],
)
