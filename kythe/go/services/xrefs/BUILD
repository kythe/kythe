load("@//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_diff//:diffmatchpatch",
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/services/web",
        "//kythe/go/test/testutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:xref_proto_go",
    ],
)
