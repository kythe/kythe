load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "wrapper",
    srcs = [
        "gradle_build_modifier.go",
    ],
    deps = [
    ],
)

go_test(
    name = "wrapper_test",
    srcs = ["gradle_build_modifier_test.go"],
    data = glob(["testdata/**"]),
    library = ":wrapper",
    deps = [
        "@com_github_google_go_cmp//cmp:go_default_library",
    ],
)
