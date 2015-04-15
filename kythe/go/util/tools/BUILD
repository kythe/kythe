load("//tools/build_rules/go", "go_binary")

package(default_visibility = ["//visibility:public"])

action_listener(
    name = "print_extra_actions",
    extra_actions = [":printer"],
    mnemonics = [
        "Javac",
        "CppCompile",
    ],
)

extra_action(
    name = "printer",
    cmd = "$(location :print_extra_action) $(EXTRA_ACTION_FILE)",
    tools = [":print_extra_action"],
)

go_binary(
    name = "print_extra_action",
    srcs = ["print_extra_action/print_extra_action.go"],
    deps = [
        "//third_party/bazel:extra_actions_base_proto",
        "//third_party/go:protobuf",
    ],
)
