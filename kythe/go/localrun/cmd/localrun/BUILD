load("//tools:build_rules/shims.bzl", "go_binary")

package(default_visibility = ["//kythe:default_visibility"])

go_binary(
    name = "localrun",
    srcs = ["main.go"],
    deps = [
        "//kythe/go/localrun",
        "//kythe/go/storage/leveldb",
        "//kythe/go/util/datasize",
        "//kythe/go/util/log",
    ],
)
