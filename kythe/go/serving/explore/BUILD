load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(
    default_visibility = ["//kythe:default_visibility"],
    licenses = ["notice"],
)

go_library(
    name = "explore",
    srcs = ["explore.go"],
    importpath = "kythe.io/kythe/go/serving/explore",
    deps = [
        "//kythe/go/storage/table",
        "//kythe/proto:explore_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_bitbucket_creachadair_stringset//:stringset",
    ],
)

go_test(
    name = "explore_test",
    size = "small",
    srcs = ["explore_test.go"],
    library = ":explore",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/storage/table",
        "//kythe/go/test/testutil",
        "//kythe/proto:explore_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_bitbucket_creachadair_stringset//:stringset",
        "@org_golang_google_protobuf//proto",
    ],
)
