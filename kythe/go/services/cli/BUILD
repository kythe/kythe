load("@io_bazel_rules_go//go:def.bzl", "go_library")

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
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/markedsource",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:identifier_go_proto",
        "//kythe/proto:xref_go_proto",
        "@go_protobuf//:proto",
        "@go_stringset//:stringset",
        "@go_subcommands//:subcommands",
    ],
)
