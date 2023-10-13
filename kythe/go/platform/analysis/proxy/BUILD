load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "proxy",
    srcs = ["proxy.go"],
    importpath = "kythe.io/kythe/go/platform/analysis/proxy",
    deps = [
        "//kythe/go/util/log",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "proxy_test",
    size = "small",
    srcs = ["proxy_test.go"],
    library = ":proxy",
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/compare",
        "//kythe/go/util/log",
        "//kythe/go/util/schema/facts",
        "//kythe/proto:analysis_go_proto",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:storage_go_proto",
        "@org_golang_google_protobuf//encoding/protojson",
        "@org_golang_google_protobuf//proto",
    ],
)
