load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "info",
    srcs = [
        "info.go",
    ],
    deps = [
        "//kythe/go/platform/kzip",
        "//kythe/go/util/compare",
        "//kythe/proto:analysis_go_proto",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
    ],
)

go_test(
    name = "info_test",
    srcs = ["info_test.go"],
    library = ":info",
    deps = [
        "//kythe/proto:analysis_go_proto",
        "@com_github_golang_protobuf//proto:go_default_library",
    ],
)
