load("//:visibility.bzl", "PUBLIC_VISIBILITY")
load("//tools:build_rules/shims.bzl", "go_library")

go_library(
    name = "treeset",
    srcs = ["treeset.go"],
    importpath = "kythe.io/kythe/go/extractors/bazel/treeset",
    visibility = [PUBLIC_VISIBILITY],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
        "@org_bitbucket_creachadair_stringset//:stringset",
    ],
)
