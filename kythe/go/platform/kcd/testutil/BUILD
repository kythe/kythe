load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "testutil",
    srcs = ["testutil.go"],
    importpath = "kythe.io/kythe/go/platform/kcd/testutil",
    deps = [
        "//kythe/go/platform/kcd",
        "//kythe/go/platform/kcd/kythe",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//proto",
    ],
)
