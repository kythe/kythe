load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_protobuf//:proto",
        "@go_shell//:shell",
        "@go_x_net//:context",
        "//kythe/go/extractors/golang",
        "//kythe/go/extractors/govname",
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kindex",
        "//kythe/go/storage/vnameutil",
        "//kythe/proto:analysis_proto_go",
        "//kythe/proto:storage_proto_go",
        "//third_party/bazel:extra_actions_base_proto_go",
    ],
)
