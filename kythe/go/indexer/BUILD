load("//tools:build_rules/go.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "indexer",
    srcs = [
        "facts.go",
        "indexer.go",
    ],
    deps = [
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "@go_x_tools//:go/gcimporter15",
        "//kythe/go/extractors/govname",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
)

go_test(
    name = "indexer_test",
    size = "small",
    srcs = ["indexer_test.go"],
    # TODO(fromberger): Build this with a library rule.
    data = [":testdata/foo.a"],
    library = ":indexer",
    deps = ["@go_protobuf//:proto"],
)
