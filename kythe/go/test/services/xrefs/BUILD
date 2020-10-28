load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = ["xrefs.go"],
    deps = [
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/markedsource",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "//kythe/proto:xref_go_proto",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
