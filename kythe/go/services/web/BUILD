load("//tools:build_rules/shims.bzl", "go_library")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "web",
    srcs = ["web.go"],
    importpath = "kythe.io/kythe/go/services/web",
    deps = [
        "//kythe/go/util/httpencoding",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
    ],
)
