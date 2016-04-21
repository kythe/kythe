load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "@go_x_text//:encoding",
        "@go_x_text//:encoding/unicode",
        "@go_x_text//:transform",
        "//kythe/go/test/testutil",
    ],
    deps = [
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:xref_proto_go",
    ],
)
