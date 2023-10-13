load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "disksort",
    srcs = ["disksort.go"],
    importpath = "kythe.io/kythe/go/util/disksort",
    visibility = [
        "//kythe:default_visibility",
        "//third_party/beam:__pkg__",
    ],
    deps = [
        "//kythe/go/platform/delimited",
        "//kythe/go/util/log",
        "//kythe/go/util/sortutil",
        "@com_github_golang_snappy//:snappy",
    ],
)

go_test(
    name = "disksort_test",
    size = "small",
    srcs = ["disksort_test.go"],
    library = ":disksort",
    visibility = ["//visibility:private"],
)
