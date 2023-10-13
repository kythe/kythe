load("//tools:build_rules/shims.bzl", "go_binary", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

alias(
    name = "kythe_languageserver",
    actual = "//kythe/go/languageserver/bin",
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
    importpath = "kythe.io/kythe/go/languageserver",
    deps = [
        "//kythe/go/languageserver/pathmap",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/markedsource",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_sergi_go_diff//diffmatchpatch",
        "@com_github_sourcegraph_go_langserver//pkg/lsp",
        "@com_github_sourcegraph_jsonrpc2//:jsonrpc2",
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
    library = ":languageserver",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_sourcegraph_go_langserver//pkg/lsp",
    ],
)
