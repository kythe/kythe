load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    deps = [
        "@go_protobuf//:proto",
        "//kythe/go/storage/vnameutil",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:storage_proto_go",
    ],
)
