load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "table",
    srcs = ["table.go"],
    importpath = "kythe.io/kythe/go/storage/table",
    deps = [
        "//kythe/go/storage/keyvalue",
        "@org_golang_google_protobuf//proto",
    ],
)
