load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "kytheuri",
    srcs = [
        "escape.go",
        ":uri.go",
    ],
    deps = ["//kythe/proto:storage_proto_go"],
)

go_test(
    name = "kytheuri_test",
    size = "small",
    srcs = [
        "bench_test.go",
        "uri_test.go",
    ],
    library = "kytheuri",
    visibility = ["//visibility:private"],
    deps = ["@go_protobuf//:proto"],
)
