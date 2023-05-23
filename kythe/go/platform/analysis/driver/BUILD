load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "driver",
    srcs = ["driver.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/go/util/log",
        "//kythe/proto:analysis_go_proto",
        "@com_github_pkg_errors//:go_default_library",
    ],
)

go_test(
    name = "driver_test",
    size = "small",
    srcs = ["driver_test.go"],
    library = "driver",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/proto:storage_go_proto",
    ],
)
