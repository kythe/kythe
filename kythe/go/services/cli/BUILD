load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cli",
    srcs = [
        "cli.go",
        "command_decor.go",
        "command_diagnostics.go",
        "command_docs.go",
        "command_edges.go",
        "command_identifiers.go",
        "command_ls.go",
        "command_nodes.go",
        "command_source.go",
        "commands_xrefs.go",
    ],
    importpath = "kythe.io/kythe/go/services/cli",
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/filetree",
        "//kythe/go/services/graph",
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/identifiers",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/markedsource",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_google_subcommands//:subcommands",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_google_protobuf//proto",
    ],
)
