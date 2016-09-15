load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/inmemory",
        "//kythe/go/util/kytheuri",
        "//kythe/go/test/testutil",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_x_net//:context",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/encoding/text",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/go/util/schema/nodes",
        "//kythe/proto:storage_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_stringset//:stringset",
        "@go_x_net//:context",
    ],
)
