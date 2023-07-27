load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "kzipdb",
    srcs = ["kzipdb.go"],
    importpath = "kythe.io/kythe/go/platform/kcd/kzipdb",
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/platform/kcd/kythe",
        "//kythe/go/platform/kzip",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "kzipdb_test",
    srcs = ["kzipdb_test.go"],
    library = ":kzipdb",
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/platform/kcd/kythe",
        "//kythe/go/platform/kzip",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@com_github_google_go_cmp//cmp",
    ],
)
