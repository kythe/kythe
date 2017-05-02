load("//tools:build_rules/go.bzl", "go_package_library")

package(default_visibility = ["//kythe:default_visibility"])

go_package_library(
    name = "cli",
    srcs = glob(["*.go"]),
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/filetree",
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/util/build",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/markedsource",
        "//kythe/go/util/schema/edges",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:common_proto_go",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:graph_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_protobuf//:proto",
        "@go_stringset//:stringset",
        "@go_subcommands//:subcommands",
    ],
)
