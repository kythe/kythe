load("//tools:build_rules/shims.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "link",
    srcs = ["link.go"],
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:link_go_proto",
        "//kythe/proto:xref_go_proto",
        "@go_grpc//:codes",
        "@go_grpc//:status",
        "@go_protobuf//:proto",
        "@go_stringset//:stringset",
        "@go_sync//:errgroup",
        "@go_sync//:semaphore",
    ],
)

go_test(
    name = "link_test",
    srcs = ["link_test.go"],
    library = ":link",
)
