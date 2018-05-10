load("//tools:build_rules/shims.bzl", "go_test", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "assemble",
    srcs = ["assemble.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/stream",
        "//kythe/go/util/encoding/text",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/pager",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/go/util/schema/tickets",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:internal_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)

go_test(
    name = "assemble_test",
    size = "small",
    srcs = ["assemble_test.go"],
    library = "assemble",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
