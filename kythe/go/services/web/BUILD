load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "web",
    srcs = ["web.go"],
    deps = [
        "//kythe/go/util/httpencoding",
        "@org_golang_google_protobuf//encoding/protojson:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)
