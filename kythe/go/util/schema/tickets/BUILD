load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
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
