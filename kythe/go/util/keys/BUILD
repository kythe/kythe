load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "keys",
    srcs = ["keys.go"],
    deps = [
        "//kythe/proto:storage_go_proto",
        "@com_github_google_orderedcode//:go_default_library",
    ],
)

go_test(
    name = "keys_test",
    srcs = ["keys_test.go"],
    library = ":keys",
    deps = ["//kythe/go/util/compare"],
)
