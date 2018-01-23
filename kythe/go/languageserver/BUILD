load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_binary", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kythe_languageserver",
    srcs = ["bin/main.go"],
    deps = [
        ":languageserver",
        "//kythe/go/services/xrefs",
        "//kythe/proto:xref_go_proto",
        "@go_jsonrpc2//:go_default_library",
        "@go_langserver//:pkg/lsp",
    ],
)

go_library(
    name = "languageserver",
    srcs = [
        "document.go",
        "handler.go",
        "languageserver.go",
        "settingsworkspace.go",
        "workspace.go",
    ],
    deps = [
        "//kythe/go/languageserver/pathmap",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/markedsource",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:xref_go_proto",
        "@go_diff//:diffmatchpatch",
        "@go_jsonrpc2//:go_default_library",
        "@go_langserver//:pkg/lsp",
        "@go_protobuf//:jsonpb",
        "@go_protobuf//:proto",
    ],
)

go_test(
    name = "languageserver_test",
    size = "small",
    srcs = [
        "document_test.go",
        "languageserver_test.go",
        "workspace_test.go",
    ],
    library = "languageserver",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
    ],
)
