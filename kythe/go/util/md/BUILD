load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "md",
    srcs = ["md.go"],
    importpath = "kythe.io/kythe/go/util/md",
)

go_test(
    name = "md_test",
    srcs = ["md_test.go"],
    library = ":md",
    deps = ["@com_github_google_go_cmp//cmp"],
)
