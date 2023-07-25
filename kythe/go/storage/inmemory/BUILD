load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "inmemory",
    srcs = ["inmemory.go"],
    importpath = "kythe.io/kythe/go/storage/inmemory",
    deps = [
        "//kythe/go/services/graphstore",
        "//kythe/go/storage/keyvalue",
        "//kythe/go/util/compare",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "inmemory_test",
    srcs = ["inmemory_test.go"],
    library = ":inmemory",
    deps = [
        "//kythe/go/storage/keyvalue",
        "@com_github_google_go_cmp//cmp",
    ],
)
