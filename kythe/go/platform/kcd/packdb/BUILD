load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "packdb",
    srcs = ["packdb.go"],
    deps = [
        "//kythe/go/platform/indexpack",
        "//kythe/go/platform/kcd",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "packdb_test",
    size = "small",
    srcs = ["packdb_test.go"],
    library = "packdb",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/platform/kcd/kythe",
        "//kythe/go/platform/kcd/testutil",
    ],
)
