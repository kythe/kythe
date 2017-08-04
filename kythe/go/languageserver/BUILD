load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kythe_languageserver",
    srcs = ["bin/main.go"],
    deps = [
        ":languageserver",
        "//kythe/proto:xref_proto_go",
        "//kythe/go/services/xrefs",
        "@go_langserver//:pkg/lsp",
        "@go_jsonrpc2//:go_default_library",
    ],
)

go_library(
    name = "languageserver",
    srcs = [
        "languageserver.go",
        "handler.go",
        "document.go",
        "path.go",
        "settings.go",
    ],
    deps = [
        "//kythe/go/languageserver/pathmap",
        "//kythe/go/util/kytheuri",
        "//kythe/go/services/xrefs",
        "//kythe/proto:xref_proto_go",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:graph_proto_go",
        "@go_diff//:diffmatchpatch",
        "@go_protobuf//:proto",
        "@go_protobuf//:jsonpb",
        "@go_langserver//:pkg/lsp",
        "@go_jsonrpc2//:go_default_library"
    ],
)

go_test(
    name = "languageserver_test",
    size = "small",
    srcs = ["document_test.go", "path_test.go", "languageserver_test.go"],
    library = "languageserver",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
    ],
)
