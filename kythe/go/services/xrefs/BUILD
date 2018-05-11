load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = ["xrefs.go"],
    deps = [
        "//kythe/go/services/web",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@go_diff//:diffmatchpatch",
        "@go_grpc//:codes",
        "@go_grpc//:status",
        "@go_stringset//:stringset",
    ],
)

go_test(
    name = "xrefs_test",
    size = "small",
    srcs = ["xrefs_test.go"],
    library = "xrefs",
    visibility = ["//visibility:private"],
)
