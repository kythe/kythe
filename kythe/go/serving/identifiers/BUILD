load("//tools:build_rules/go.bzl", "go_package_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "identifiers",
    srcs = ["identifiers.go"],
    deps = [
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema/tickets",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:identifier_proto_go",
        "//kythe/proto:internal_proto_go",
        "//kythe/proto:serving_proto_go",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "identifiers_test",
    size = "small",
    srcs = ["identifiers_test.go"],
    library = "identifiers",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "@go_protobuf//:proto",
        "@go_x_text//:encoding",
        "@go_x_text//:encoding/unicode",
        "@go_x_text//:transform",
    ],
)
