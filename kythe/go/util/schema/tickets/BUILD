load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "tickets",
    srcs = ["tickets.go"],
    deps = ["//kythe/go/util/kytheuri"],
)

go_test(
    name = "tickets_test",
    size = "small",
    srcs = ["tickets_test.go"],
    library = "tickets",
)
