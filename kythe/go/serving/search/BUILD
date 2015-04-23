load("//tools/build_rules/go", "go_package")

package(default_visibility = ["//kythe:default_visibility"])

go_package(
    test_deps = [
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:serving_proto",
        "//kythe/proto:storage_proto",
    ],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:serving_proto",
        "//kythe/proto:storage_proto",
    ],
)
