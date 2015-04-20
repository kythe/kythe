load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "search",
    srcs = [
        "search.go",
    ],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:serving_proto",
        "//kythe/proto:storage_proto",
    ],
)

go_test(
    name = "search_test",
    srcs = [
        "search_test.go",
    ],
    library = ":search",
    deps = [
        "//kythe/go/storage/leveldb",
        "//kythe/go/storage/table",
        "//kythe/go/util/kytheuri",
        "//kythe/proto:serving_proto",
        "//kythe/proto:storage_proto",
    ],
)
