load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(
    default_visibility = ["//kythe:default_visibility"],
    licenses = ["notice"],
)

go_library(
    name = "explore",
    srcs = ["explore.go"],
    deps = [
        "//kythe/go/services/explore",
        "//kythe/go/storage/table",
        "//kythe/proto:explore_go_proto",
        "//kythe/proto:serving_go_proto",
        "@org_golang_google_protobuf//proto:go_default_library",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_google_grpc//codes:go_default_library",
        "@org_golang_google_grpc//status:go_default_library",
    ],
)

go_test(
    name = "explore_test",
    size = "small",
    srcs = ["explore_test.go"],
    library = "explore",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/kytheuri",
        "@org_bitbucket_creachadair_stringset//:go_default_library",
        "@org_golang_x_text//encoding:go_default_library",
        "@org_golang_x_text//encoding/unicode:go_default_library",
        "@org_golang_x_text//transform:go_default_library",
    ],
)
