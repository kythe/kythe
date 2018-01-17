load("@io_bazel_rules_go//go:def.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "print_test_status",
    srcs = ["print_test_status.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//third_party/bazel:test_status_go_proto",
        "@go_protobuf//:proto",
    ],
)
