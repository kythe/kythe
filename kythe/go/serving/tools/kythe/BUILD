load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "kythe",
    srcs = [
        "kythe.go",
        "kythe_commands.go",
        "kythe_display.go",
    ],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/services/filetree",
        "//kythe/go/services/web",
        "//kythe/go/services/xrefs",
        "//kythe/go/serving/api",
        "//kythe/go/util/build",
        "//kythe/go/util/kytheuri",
        "//kythe/go/util/schema",
        "//kythe/go/util/stringset",
        "//kythe/proto:filetree_proto_go",
        "//kythe/proto:xref_proto_go",
        "@go_protobuf//:proto",
        "@go_x_net//:context",
    ],
)
