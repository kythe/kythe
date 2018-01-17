load("@io_bazel_rules_go//go:def.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kytheuri",
    srcs = [
        "escape.go",
        ":uri.go",
    ],
    deps = ["//kythe/proto:storage_go_proto"],
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
    deps = [
        "//kythe/proto:storage_go_proto",
        "@go_protobuf//:proto",
    ],
)
