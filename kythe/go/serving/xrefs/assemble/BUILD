load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = ["//kythe/go/test/testutil"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/encoding/text",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/pager",
        "//kythe/go/util/schema",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_x_net//:context",
    ],
)
