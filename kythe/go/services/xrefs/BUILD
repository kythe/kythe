load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "xrefs",
    srcs = ["xrefs.go"],
    importpath = "kythe.io/kythe/go/services/xrefs",
    deps = [
        "//kythe/go/services/web",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/edges",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:xref_go_proto",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_google_grpc//codes",
        "@org_golang_google_grpc//status",
    ],
)

go_test(
    name = "xrefs_test",
    size = "small",
    srcs = ["xrefs_test.go"],
    library = ":xrefs",
    visibility = ["//visibility:private"],
    deps = ["//kythe/go/util/schema/facts"],
)
