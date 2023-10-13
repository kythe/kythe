load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "compare",
    srcs = ["compare.go"],
    importpath = "kythe.io/kythe/go/util/compare",
    deps = [
        "//kythe/proto:storage_go_proto",
        "@com_github_google_go_cmp//cmp",
        "@org_golang_google_protobuf//testing/protocmp",
    ],
)

go_test(
    name = "compare_test",
    srcs = ["compare_test.go"],
    library = ":compare",
    deps = ["//kythe/proto:storage_go_proto"],
)
