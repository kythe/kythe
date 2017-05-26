package(default_visibility = ["//kythe:default_visibility"])

load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

go_package_library(
    name = "entryset",
    srcs = ["entryset.go"],
    deps = [
        "//kythe/proto:entryset_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "entryset_test",
    size = "small",
    srcs = ["entryset_test.go"],
    library = "entryset",
    deps = ["//kythe/go/util/kytheuri"],
)
