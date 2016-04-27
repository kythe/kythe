load("//tools:build_rules/go.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

action_listener(
    name = "print_extra_actions",
    extra_actions = [":printer"],
    mnemonics = [
        "Javac",
        "CppCompile",
        "GoCompile",
    ],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "printer",
    cmd = "$(location :print_extra_action) $(EXTRA_ACTION_FILE)",
    tools = [":print_extra_action"],
)

go_binary(
    name = "scan_leveldb",
    srcs = ["scan_leveldb/scan_leveldb.go"],
    deps = [
        "@go_protobuf//:proto",
        "//kythe/go/storage/leveldb",
        "//kythe/go/util/flagutil",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
)

go_binary(
    name = "print_extra_action",
    srcs = ["print_extra_action/print_extra_action.go"],
    deps = [
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/platform/vfs",
        "//third_party/bazel:extra_actions_base_proto_go",
    ],
)

go_binary(
    name = "print_test_status",
    srcs = ["print_test_status/print_test_status.go"],
    deps = [
        "@go_protobuf//:proto",
        "@go_x_net//:context",
        "//kythe/go/platform/vfs",
        "//third_party/bazel:test_status_proto_go",
    ],
)
