load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "proxy",
    srcs = ["proxy.go"],
    deps = [
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson:go_default_library",
        "@org_golang_google_protobuf//proto:go_default_library",
    ],
)

go_test(
    name = "proxy_test",
    size = "small",
    srcs = ["proxy_test.go"],
    library = "proxy",
    deps = ["//kythe/go/util/compare"],
)
