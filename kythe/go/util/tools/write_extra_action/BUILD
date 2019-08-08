load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//visibility:public"])

go_binary(
    name = "write_extra_action",
    srcs = ["write_extra_action.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
