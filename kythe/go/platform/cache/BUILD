load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "cache",
    srcs = [
        "cache.go",
    ],
    deps = [
        "//kythe/go/platform/analysis",
    ],
)

go_test(
    name = "cache_test",
    srcs = [
        "cache_test.go",
    ],
    library = ":cache",
)
