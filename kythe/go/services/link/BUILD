load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "link",
    srcs = ["link.go"],
    importpath = "kythe.io/kythe/go/services/link",
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:link_go_proto",
        "//kythe/proto:xref_go_proto",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_google_grpc//codes",
        "@org_golang_google_grpc//status",
        "@org_golang_google_protobuf//encoding/prototext",
        "@org_golang_google_protobuf//proto",
        "@org_golang_x_sync//errgroup",
        "@org_golang_x_sync//semaphore",
    ],
)

go_test(
    name = "link_test",
    srcs = ["link_test.go"],
    library = ":link",
    deps = [
        "//kythe/proto:common_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:link_go_proto",
        "//kythe/proto:xref_go_proto",
        "@org_golang_google_grpc//codes",
        "@org_golang_google_grpc//status",
        "@org_golang_google_protobuf//proto",
    ],
)
