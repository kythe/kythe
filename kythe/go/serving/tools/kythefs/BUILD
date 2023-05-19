load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kythefs",
    srcs = ["kythefs.go"],
    deps = [
        "//kythe/go/serving/api",
        "//kythe/go/util/flagutil",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:filetree_go_proto",
        "//kythe/proto:graph_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_hanwen_go_fuse//fuse:go_default_library",
        "@com_github_hanwen_go_fuse//fuse/nodefs:go_default_library",
        "@com_github_hanwen_go_fuse//fuse/pathfs:go_default_library",
        "@org_golang_x_net//http2:go_default_library",
    ],
)
