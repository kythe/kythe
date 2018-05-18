load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "smoke",
    srcs = [
        "smoke.go",
    ],
    deps = [
        "//kythe/go/extractors/config",
        "//kythe/go/platform/kindex",
    ],
)

go_test(
    name = "smoke_test",
    srcs = ["smoke_test.go"],
    library = ":smoke",
    deps = [
        "//kythe/go/extractors/config",
        "//kythe/go/platform/kindex",
    ],
)
