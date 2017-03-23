load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "proxy",
    srcs = ["proxy.go"],
    deps = [
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
)

go_test(
    name = "proxy_test",
    srcs = ["proxy_test.go"],
    library = "proxy",
)
