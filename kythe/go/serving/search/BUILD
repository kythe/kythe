load("//tools:build_rules/go.bzl", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
    ],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:serving_proto_go",
        "//kythe/proto:storage_proto_go",
        "@go_x_net//:context",
    ],
)
