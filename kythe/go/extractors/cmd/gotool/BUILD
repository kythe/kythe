load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "gotool",
    srcs = [
        "gotool.go",
    ],
    deps = [
        "@go_x_net//:context",
        "//kythe/go/extractors/golang",
        "//kythe/go/platform/indexpack",
        "//kythe/proto:analysis_proto_go",
    ],
)
