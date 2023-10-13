load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "flagutil",
    srcs = ["flagutil.go"],
    importpath = "kythe.io/kythe/go/util/flagutil",
    deps = [
        "//kythe/go/util/build",
        "@org_bitbucket_creachadair_stringset//:stringset",
    ],
)
