load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "driver",
    srcs = ["driver.go"],
    deps = [
        "//kythe/go/platform/analysis",
        "//kythe/proto:analysis_proto_go",
        "@go_errors//:errors",
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
        "//kythe/proto:storage_proto_go",
    ],
)
