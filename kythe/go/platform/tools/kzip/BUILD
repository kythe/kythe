load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kzip",
    srcs = ["kzip.go"],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/platform/vfs",
        "//kythe/go/util/flagutil",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
