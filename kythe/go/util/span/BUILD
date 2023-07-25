load("//tools:build_rules/shims.bzl", "go_library", "go_test")

package(default_visibility = ["//kythe:default_visibility"])

go_library(
    name = "span",
    srcs = ["span.go"],
    importpath = "kythe.io/kythe/go/util/span",
    deps = [
        "//kythe/go/util/log",
        "//kythe/proto:common_go_proto",
        "//kythe/proto:serving_go_proto",
        "//kythe/proto:xref_go_proto",
        "@com_github_sergi_go_diff//diffmatchpatch",
        "@org_golang_google_protobuf//proto",
    ],
)

go_test(
    name = "span_test",
    size = "small",
    srcs = ["span_test.go"],
    library = ":span",
    visibility = ["//visibility:private"],
    deps = [
        "//kythe/go/test/testutil",
        "//kythe/go/util/compare",
        "//kythe/proto:common_go_proto",
        "@com_github_google_go_cmp//cmp/cmpopts",
        "@org_golang_google_protobuf//proto",
    ],
)
