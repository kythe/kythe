load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "link",
    srcs = ["link.go"],
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:link_go_proto",
        "//kythe/proto:xref_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_google_grpc//codes:go_default_library",
        "@org_golang_google_grpc//status:go_default_library",
        "@org_golang_google_protobuf//encoding/prototext:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
        "@org_golang_x_sync//errgroup:go_default_library",
        "@org_golang_x_sync//semaphore:go_default_library",
    ],
)

go_test(
    name = "link_test",
    srcs = ["link_test.go"],
    library = ":link",
)
