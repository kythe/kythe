load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "proxy",
    srcs = ["proxy.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/gsutil",
        "//kythe/proto:storage_proto_go",
    ],
)

go_test(
    name = "proxy_test",
    size = "small",
    srcs = ["proxy_test.go"],
    library = "proxy",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)
