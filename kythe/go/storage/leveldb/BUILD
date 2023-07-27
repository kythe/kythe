load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "leveldb",
    srcs = ["leveldb.go"],
    importpath = "kythe.io/kythe/go/storage/leveldb",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/keyvalue",
        "@com_github_jmhodges_levigo//:levigo",
    ],
)

go_test(
    name = "leveldb_test",
    size = "small",
    srcs = ["leveldb_test.go"],
    library = ":leveldb",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/services/graphstore",
        "//kythe/go/test/storage/keyvalue",
    ],
)
