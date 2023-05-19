load("//tools:build_rules/shims.bzl", "go_binary", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kythe_languageserver",
    srcs = ["bin/main.go"],
    deps = [
        ":languageserver",
        "//kythe/go/services/xrefs",
        "//kythe/proto:xref_go_proto",
        "@com_github_sourcegraph_go_langserver//pkg/lsp:go_default_library",
        "@com_github_sourcegraph_jsonrpc2//:go_default_library",
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
        "//kythe/go/util/log",
        "//kythe/go/util/markedsource",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_golang_protobuf//jsonpb:go_default_library_gen",
        "@com_github_sergi_go_diff//diffmatchpatch:go_default_library",
        "@com_github_sourcegraph_go_langserver//pkg/lsp:go_default_library",
        "@com_github_sourcegraph_jsonrpc2//:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
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
