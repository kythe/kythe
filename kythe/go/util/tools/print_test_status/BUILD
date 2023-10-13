load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

genrule(
    name = "copy_print_test_status",
    srcs = [":print_test_status_bin"],
    outs = ["print_test_status"],
    cmd = "cp $< $@",
    output_to_bindir = True,
)

go_binary(
    name = "print_test_status_bin",
    srcs = ["print_test_status.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
        "//third_party/bazel:test_status_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
