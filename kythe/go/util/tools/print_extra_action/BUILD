load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

action_listener(
    name = "print_extra_actions",
    extra_actions = [":printer"],
    mnemonics = [
        "ObjcCompile",
        "CppCompile",
        "GoCompilePkg",
        "JavaIjar",
        "Javac",
    ],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "printer",
    cmd = "$(location :print_extra_action) $(EXTRA_ACTION_FILE)",
    tools = [":print_extra_action"],
)

go_binary(
    name = "print_extra_action",
    srcs = ["print_extra_action.go"],
    deps = [
        "//kythe/go/platform/vfs",
        "//kythe/go/util/log",
        "//third_party/bazel:extra_actions_base_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
