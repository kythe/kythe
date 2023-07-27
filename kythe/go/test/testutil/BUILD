load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "testutil",
    srcs = ["testutil.go"],
    importpath = "kythe.io/kythe/go/test/testutil",
    deps = [
        "//kythe/go/util/compare",
        "@com_github_google_go_cmp//cmp",
        "@io_k8s_sigs_yaml//:yaml",
    ],
)
