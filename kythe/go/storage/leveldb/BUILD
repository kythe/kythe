load("//tools:build_rules/testing.bzl", "go_test")
load("@io_bazel_rules_go//go:def.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "leveldb",
    srcs = ["leveldb.go"],
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/gsutil",
        "//kythe/go/storage/keyvalue",
        "@go_levigo//:levigo",
    ],
)

go_test(
    name = "leveldb_test",
    size = "small",
    srcs = ["leveldb_test.go"],
    library = "leveldb",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/services/graphstore",
        "//kythe/go/test/storage/keyvalue",
    ],
)
