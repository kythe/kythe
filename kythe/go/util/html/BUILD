load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "html",
    srcs = ["html.go"],
    importpath = "kythe.io/kythe/go/util/html",
    deps = [
        "//kythe/go/util/log",
        "@org_golang_x_net//html",
    ],
)

go_test(
    name = "html_test",
    size = "small",
    srcs = ["html_test.go"],
    library = ":html",
    visibility = ["//visibility:private"],
    deps = ["@org_golang_x_net//html"],
)
