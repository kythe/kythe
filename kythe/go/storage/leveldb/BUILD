load("//tools/build_rules/go", "go_library", "go_test")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "leveldb",
    srcs = [
        "leveldb.go",
    ],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/keyvalue",
        "//third_party/go:levigo",
    ],
)

go_test(
    name = "leveldb_test",
    srcs = [
        "leveldb_test.go",
    ],
    library = ":leveldb",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/services/graphstore/compare",
        "//kythe/go/storage/keyvalue",
        "//kythe/proto:storage_proto",
    ],
)
