load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "cli",
    srcs = glob(["*.go"]),
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/filetree",
        "//kythe/go/services/graph",
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/identifiers",
        "//kythe/go/util/build",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/markedsource",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
        "@com_github_google_subcommands//:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)
