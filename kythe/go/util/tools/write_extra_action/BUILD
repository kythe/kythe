load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//visibility:public"])

go_binary(
    name = "write_extra_action",
    srcs = ["write_extra_action.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@org_golang_google_protobuf//encoding/prototext:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
