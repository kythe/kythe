load("//tools:build_rules/shims.bzl", "go_library", "go_test")

go_library(
    name = "java_wrapper",
    srcs = ["java_wrapper.go"],
    visibility = [
        "//kythe/extractors/openjdk21:__subpackages__",
    ],
    deps = [
        "//kythe/go/util/log",
        "@org_bitbucket_creachadair_shell//:shell",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_x_sys//unix",
    ],
)

go_test(
    name = "java_wrapper_test",
    srcs = ["java_wrapper_test.go"],
    library = ":java_wrapper",
)
